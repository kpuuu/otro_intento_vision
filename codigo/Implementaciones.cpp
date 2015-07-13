void En(Mat depth, int f)
{
	int cols = depth.cols, rows= depth.rows;
	Mat grad_x,grad_y;
	int ddepth = CV_16S;
	int scale = 1;
	int delta = 0;
	Sobel( depth, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
	Sobel( depth, grad_y, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
	for(int i=0;i<rows;i++)
	{
		for(int j=0;j<cols;j++)
		{
			
			int Tx_x=(-1/f)*(depth.at<uchar>(j,i)+(j*(grad_x.at<uchar>(j, i)));//en la posiciom
			int Tx_y=(-1/f)*i*(grad_x.at<uchar>(j, i));//en la posicion
			int Tx_z=grad_x.at<uchar>(j, i);//en la posicion
			int Ty_x=(-1/f)*(j*grad_y.at<uchar>(j, i));
			int Ty_y=(-1/f)*(depth.at<uchar>(j, i)+(i*(grad_x.at<uchar>(j, i)));
			int Ty_z=grad_y.at<uchar>(j, i);
			//typedef Point3_<float> tx(Tx_x,Tx_y,Tx_z);
			//typedef Point3_<float> ty(Ty_x,Ty_y,Ty_z);
			
		}
	}

}
void N(Mat norm, Mat t_norm,int posx,int posy)
{
	int cols = norm.cols, rows= norm.rows,int suma=0;
	for (int i = posx; i <posx+ 3; ++i)
	{
		for (int j = posy; j < posy+3; ++j)
		{
			suma= 
		}
	}
}

void Ed(Mat depth, int f)
{
	int cols = depth.cols, rows= depth.rows;
	int suma=0,w_0=0.9;
	int Up_z=-1;
	for(int i=0;i<rows;i++)
	{
		for(int j=2;j<cols;j++)
		{
			int Up_x=-j/f;
			int Up_y=-i/f;
			int wp=w_0+(1-w_0)*N.at<uchar>(j,i); //FALTAN LOS VALORES PROPIOS

		}
	}
}

void E_cost(Mat Ed,Mat En,MatEs)
{
	// FALTAN LOS LAMBDAS
	Mat E=Ed+En+Es;
}