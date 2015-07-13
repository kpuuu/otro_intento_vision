#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <math.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/background_segm.hpp>

using namespace cv;
using namespace std;



template <typename T>
int sgn(T val) 
{
  return (T(0) < val) - (val < T(0));
}

void exportMesh(cv::Mat Depth, cv::Mat Normals, cv::Mat texture) 
{
  /* writing obj for export */
  std::ofstream objFile, mtlFile;
  objFile.open("export.obj");
  int width = Depth.cols;
  int height = Depth.rows;
  /* vertices, normals, texture coords */
  objFile << "mtllib export.mtl" << std::endl;

  for (int y = 0; y < height; y++) 
  {
    for (int x = 0; x < width; x++) 
    {
      objFile << "v " << x << " " << y << " "
              << Depth.at<float>(cv::Point(x, y)) << std::endl;
      objFile << "vt " << x / (width - 1.0f) << " " << (1.0f - y) / height
              << " "
              << "0.0" << std::endl;
      objFile << "vn " << (float)Normals.at<cv::Vec3b>(y, x)[0] << " "
              << (float)Normals.at<cv::Vec3b>(y, x)[1] << " "
              << (float)Normals.at<cv::Vec3b>(y, x)[2] << std::endl;
    }
  }

  /* faces */
  objFile << "usemtl picture" << std::endl;

  for (int y = 0; y < height - 1; y++) 
  {
    for (int x = 0; x < width - 1; x++) 
    {
      int f1 = x + y * width + 1;
      int f2 = x + y * width + 2;
      int f3 = x + (y + 1) * width + 1;
      int f4 = x + (y + 1) * width + 2;
      objFile << "f " << f1 << "/" << f1 << "/" << f1 << " ";
      objFile << f2 << "/" << f2 << "/" << f2 << " ";
      objFile << f3 << "/" << f3 << "/" << f3 << std::endl;
      objFile << "f " << f2 << "/" << f2 << "/" << f2 << " ";
      objFile << f4 << "/" << f4 << "/" << f4 << " ";
      objFile << f3 << "/" << f3 << "/" << f3 << std::endl;
    }
  }

  /* texture */
  cv::imwrite("export.jpg", texture);
  mtlFile.open("export.mtl");
  mtlFile << "newmtl picture" << std::endl;
  mtlFile << "map_Kd export.jpg" << std::endl;
  objFile.close();
  mtlFile.close();
}

Mat imageMask(vector<Mat> camImages, int numPics, Mat ambient) 
{
 Mat resta= Mat::eye(camImages[0].rows,camImages[0].cols,camImages[0].type());
 for (int i = 0; i < numPics; ++i)
  {
    resta=resta+camImages[i];
  }
  resta=resta-ambient;
 // cout<<resta.type()<<endl;
  double thresh = 100;
  double maxValue = 255; 
  threshold(resta,resta, thresh, maxValue, THRESH_BINARY);
  //imshow("RESTA", resta);
 // imshow("pls",result);
  return resta;
}


Mat computeNormals(vector<Mat> camImages,Mat Mask = Mat()) 
{
  int height = camImages[0].rows;
  int width = camImages[0].cols;
  int numImgs = camImages.size();
  /* populate A */
  Mat A(height * width, numImgs, CV_32FC1, Scalar::all(0));

  for (int k = 0; k < numImgs; k++) 
  {
    int idx = 0;

    for (int i = 0; i < height; i++)
     {
      for (int j = 0; j < width; j++)
      {
        A.at<float>(idx++, k) = camImages[k].data[i * width + j] *sgn(Mask.at<uchar>(cv::Point(j, i)));
      }
    }
  }

  /* speeding up computation, SVD from A^TA instead of AA^T */
  Mat U, S, Vt;
  SVD::compute(A.t(), S, U, Vt, SVD::MODIFY_A);
  Mat EV = Vt.t();
  Mat N(height, width, CV_8UC3, Scalar::all(0));
  int idx = 0;
//  cout<<"tamano de u es"<<U.size()<<endl;
  //cout<<"tamano de S es"<<S.size()<<endl;
  //cout<<"tamano de VT es"<<Vt.size()<<endl;
  for (int i = 0; i < height; i++) 
  {
    for (int j = 0; j < width; j++) 
    {
      //cuando estamos en un lugar negor de la mascara y el pixel queda azul
      if (Mask.at<uchar>(Point(j, i)) == 0) 
      {
          N.at<Vec3b>(i, j) = Vec3b(0, 0, 0);
      } 
      //Se le agrega una componente de colores de las distintos valores
      else 
      {
          float rSxyz = 1.0f / sqrt(EV.at<float>(idx, 0) * EV.at<float>(idx, 0) +
                                  EV.at<float>(idx, 1) * EV.at<float>(idx, 1) +
                                  EV.at<float>(idx, 2) * EV.at<float>(idx, 2));
          /*EV contiene los valores propios que se calculan con atravez del SVD
          *los que representanlos componentes de la superficie normal para cada pixel.*/
         
          float sz = 128.0f + 127.0f * sgn(EV.at<float>(idx, 0)) * fabs(EV.at<float>(idx, 0)) * rSxyz;
          float sx = 128.0f + 127.0f * sgn(EV.at<float>(idx, 1)) * fabs(EV.at<float>(idx, 1)) * rSxyz;
          float sy = 128.0f + 127.0f * sgn(EV.at<float>(idx, 2)) * fabs(EV.at<float>(idx, 2)) * rSxyz;
          N.at<Vec3b>(i, j) = cv::Vec3b(sx, sy, sz);
      }

      idx += 1;
    }
  }

  return N;
}

void updateHeights(cv::Mat &Normals, cv::Mat &Z, int iterations) 
{
  for (int k = 0; k < iterations; k++) 
  {
    for (int i = 1; i < Normals.rows - 1; i++) 
    {
      for (int j = 1; j < Normals.cols - 1; j++) 
      {
        float zU = Z.at<float>(cv::Point(j, i - 1));
        float zD = Z.at<float>(cv::Point(j, i + 1));
        float zL = Z.at<float>(cv::Point(j - 1, i));
        float zR = Z.at<float>(cv::Point(j + 1, i));
        float nxC = Normals.at<cv::Vec3b>(cv::Point(j, i))[0];
        float nyC = Normals.at<cv::Vec3b>(cv::Point(j, i))[1];
        float nxU = Normals.at<cv::Vec3b>(cv::Point(j, i - 1))[0];
        float nyU = Normals.at<cv::Vec3b>(cv::Point(j, i - 1))[1];
        float nxD = Normals.at<cv::Vec3b>(cv::Point(j, i + 1))[0];
        float nyD = Normals.at<cv::Vec3b>(cv::Point(j, i + 1))[1];
        float nxL = Normals.at<cv::Vec3b>(cv::Point(j - 1, i))[0];
        float nyL = Normals.at<cv::Vec3b>(cv::Point(j - 1, i))[1];
        float nxR = Normals.at<cv::Vec3b>(cv::Point(j + 1, i))[0];
        float nyR = Normals.at<cv::Vec3b>(cv::Point(j + 1, i))[1];
        int up = nxU == 0 && nyU == 0 ? 0 : 1;
        int down = nxD == 0 && nyD == 0 ? 0 : 1;
        int left = nxL == 0 && nyL == 0 ? 0 : 1;
        int right = nxR == 0 && nyR == 0 ? 0 : 1;

        if (up > 0 && down > 0 && left > 0 && right > 0) {
          Z.at<float>(cv::Point(j, i)) =
              1.0f / 4.0f * (zD + zU + zR + zL + nxU - nxC + nyL - nyC);
        }
      }
    }
  }
}

Mat cvtFloatToGrayscale(cv::Mat F, int limit = 255) 
{
  double min, max;
  cv::minMaxIdx(F, &min, &max);
  cv::Mat adjMap;
  cv::convertScaleAbs(F, adjMap, limit / max);
  return adjMap;
}

Mat localHeightfield(cv::Mat Normals) 
{
  const int pyramidLevels = 4;
  const int iterations = 2000;
  
  vector<Mat> pyrNormals;
  Mat Normalmap = Normals.clone();
  pyrNormals.push_back(Normalmap);

  for (int i = 0; i < pyramidLevels; i++) 
  {
    cv::pyrDown(Normalmap, Normalmap);
    pyrNormals.push_back(Normalmap.clone());
  }

  
  Mat Z(pyrNormals[pyramidLevels - 1].rows,pyrNormals[pyramidLevels - 1].cols, CV_32FC1, Scalar::all(0));

  for (int i = pyramidLevels - 1; i > 0; i--) 
  {
    updateHeights(pyrNormals[i], Z, iterations);
    cv::pyrUp(Z, Z);
   // std::stringstream a;
   // a << "Z"<< i;
   // imshow(a.str(),Z);
  }

  /* linear transformation of matrix values from [min,max] -> [a,b] */
  double min, max;
  minMaxIdx(Z, &min, &max);
  double a = 0.0, b = 150.0;

  for (int i = 0; i < Normals.rows; i++) 
  {
    for (int j = 0; j < Normals.cols; j++) 
    {
      Z.at<float>(cv::Point(j, i)) =
          (float)a +
          (b - a) * ((Z.at<float>(cv::Point(j, i)) - min) / (max - min));
    }
  }

  return Z;
}

void E_n(Mat Normal,Mat Depth, int f)
{
    int scale = 1;
    int delta = 0;
    int ddepth = CV_16S;
    Mat G_x1,G_y1,G_x,G_y;
    Sobel( Depth, G_x1, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
    convertScaleAbs( G_x1, G_x );
    Sobel( Depth, G_y1, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
    convertScaleAbs( G_y1, G_y );
    Mat E_n;
    for (int i = 0; i < Normal.rows; ++i)
    {
      for (int j = 0; j < Normal.cols; ++j)
      {
        Mat N_p=Mat::zeros(3,1,Depth.type());
        Vec3b N_p_aux = Normal.at<Vec3b>(i,j);
        N_p.at<float>(0,0) = N_p_aux.val[0];
        N_p.at<float>(1,0) = N_p_aux.val[1];
        N_p.at<float>(2,0) = N_p_aux.val[2];
        

        Mat T_x,T_y;
        T_x=Mat::zeros(1,3,Depth.type());
        T_y=Mat::zeros(1,3,Depth.type());
        T_x.at<float>(0,0)=-1*(1/f)*(Depth.at<float>(i,j)+(i*G_x.at<float>(i,j)));
        T_x.at<float>(0,1)=(-1)*(1/f)*j*G_x.at<float>(i,j);
        T_x.at<float>(0,2)=G_x.at<float>(i,j);
        T_y.at<float>(0,0)=(-1)*(1/f)*i*G_y.at<float>(i,j);;
        T_y.at<float>(0,1)=-1*(1/f)*(Depth.at<float>(i,j)+(j*G_y.at<float>(i,j)));;
        T_y.at<float>(0,2)=G_y.at<float>(i,j);

        E_n=E_n+((N_p*T_x)*(N_p*T_x))+((N_p*T_y)*(N_p*T_y));
      }
    }
}

void E_d(Mat DepthK, int f,Mat Normal,Mat DepthE){
  float w_0=0.9;
  Mat E_d;
  for (int i = 0; i < DepthK.rows; ++i)
  {
    for (int j = 0; j < DepthK.cols; ++j)
    {
      Mat u_p=Mat::zeros(3,1,DepthK.type());
      u_p.at<float>(0,0)=(-i)/f;
      u_p.at<float>(1,0)=(-j)/f;
      u_p.at<float>(2,0)=1;
      //_________no estoy segura de estooo revisar lo de los pixeles___
      Mat N_aux=Mat::zeros(3,1,DepthK.type());
      Vec3b N_copia = Normal.at<Vec3b>(i,j);
      N_aux.at<float>(0,0) = N_copia.val[0];
      N_aux.at<float>(1,0) = N_copia.val[1];
      N_aux.at<float>(2,0) = N_copia.val[2];
      Mat N=N_aux*N_aux.t();
      Mat E, V;
      eigen(N,E,V);
      float lamba_1=E.at<float>(0,2);
      float lamba_2=E.at<float>(0,1);
      float lamba_3=E.at<float>(0,0);

      float signo_raro= 1-((lamba_2-lamba_1)/lamba_3);
      int w_p= w_0 + ((1+w_0)*signo_raro);

      E_d+= w_p*pow(norm(u_p,NORM_L2),2)*((DepthK.at<float>(i,j)-(DepthE.at<float>(i,j)))*(DepthK.at<float>(i,j)-(DepthE.at<float>(i,j))));
    }
  }
}

void E_s(Mat Depth)
{
    int scale = 1;
    int delta = 0;
    int ddepth = CV_16S;
    Mat E_s;
    Mat gx_aux,gy_aux,g_x,g_y;
    Sobel( Depth, gx_aux, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
    convertScaleAbs( gx_aux, g_x );
    Sobel( Depth, gy_aux, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
    convertScaleAbs( gy_aux, g_y );
    Mat gx_aux2,gy_aux2,g_x2,g_y2;
    Sobel( Depth, gx_aux2, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
    convertScaleAbs( gx_aux2, g_x2 );
    Sobel( Depth, gy_aux2, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
    convertScaleAbs( gy_aux2, g_y2 );
    E_s=g_x2+g_y2;
    // Coordenadas cartesianas https://es.wikipedia.org/wiki/Operador_laplaciano

}

int main(int argc, char *argv[]) 
{

  int numPics = 4;
 
  vector<Mat> camImages;
 // vector<Mat> depth_maps;
  cout<<"1"<<endl;
  for (int i = 0; i < numPics; i++) 
  {
      std::stringstream s;
      s << "../images/" << i << ".png";
      camImages.push_back(cv::imread(s.str(), CV_LOAD_IMAGE_GRAYSCALE));
  }
/*
  for (int i = 0; i < numPics; i++) 
  {
      std::stringstream s;
      s << "../images/" << i << "_profundidad.png";
      depth_maps.push_back(cv::imread(s.str(), CV_LOAD_IMAGE_GRAYSCALE));
  }*/
  cout<<"2"<<endl;
  stringstream s;
  s<<"../images/ambiente.png";
  Mat ambient=cv::imread(s.str(),CV_LOAD_IMAGE_GRAYSCALE);
cout<<"3"<<endl;
  int size_kernel=3;
  for (int i = 0; i < camImages.size(); i++) 
  {
    stringstream s;
    s << i << ".png";
    GaussianBlur( camImages[i], camImages[i], Size( size_kernel,size_kernel), 0, 0 );
  }
 // cout<<"PRUEBA DEL SGN"<<endl;
  //int prueba=sgn(0);
  //cout<<prueba<<endl;
 // Mat profundidad=Mat::eye(depth_maps[0].rows,depth_maps[0].cols,depth_maps[0].type());;
 // for (int i = 0; i < numPics; ++i)
 // {
  //  profundidad=profundidad+camImages[i];
 // }
  /*
  double min, max;
  cv::minMaxLoc(profundidad, &min, &max);

  //__________normalizar la profundidad______________________
    for (int i = 0; i < profundidad.rows; ++i)
    {
      for (int j = 0; j < profundidad.cols; ++j)
      {
        profundidad.at<float>(i,j)=(1/max)*profundidad.at<float>(i,j);
      }
    }
  //_________________________________________________________
*/
  

  cout<<"4"<<endl;
  Mat Mask = imageMask(camImages,numPics,ambient);
  imshow("Mask", Mask);
  imwrite( "Mascara1.png", Mask );
  for (int i = 0; i < numPics; ++i)
  {
    stringstream aux;
    aux<<i;
   // imshow(aux.str(),camImages[i]);
    bitwise_and(camImages[i],Mask,camImages[i]);
    aux<<"_mask";
   // imshow(aux.str(),camImages[i]);
  }
cout<<"5 normalmap"<<endl;
  Mat S = computeNormals(camImages, Mask);
  //cout<<"S"<<S.at<Vec3b>(0,0)<<endl;
  imshow("s", S);
  Mat Normalmap;
  cvtColor(S, Normalmap, CV_BGR2RGB);

  imshow("normalmap.png", Normalmap);
  imwrite( "Normal1.png", Normalmap );
  
  cout<<"6 depth S"<<endl;
  Mat Depth = localHeightfield(S);

  //cout<<"Depth"<<Depth<<endl;
  imshow("Local Depthmap", cvtFloatToGrayscale(Depth));
  imwrite( "Profundidad.png", Depth );
  /*Mat depf = cvtFloatToGrayscale(Depth);
  Mat crepu = Mask-depf;
  crepu+=1;
  for (int i = 0; i < crepu.cols; ++i)
  {
    for (int j = 0; j < crepu.rows; ++j)
    {
      
      if(crepu.at<float>(j,i) == 0) crepu.at<float>(j,i)= 10;
    }
   
  }
    cout<<crepu<<endl;*/
  //imshow("pls",crepu);
 // imshow("Local Depthmap 2", Depth);
  exportMesh(Depth, S, camImages[0]);
  waitKey(0);
  return 0;
}
