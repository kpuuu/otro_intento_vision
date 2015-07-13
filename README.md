Proyecto Final Visión por Computador

High Quality Photometric Reconstruction using a Depth Camera

@INPROCEEDINGS{smh2014high,
  author = {Haque, Sk. Mohammadul and Chatterjee, Avishek and Govindu, Venu Madhav},
  title = {High Quality Photometric Reconstruction from a Depth Camera},
  booktitle = {2014 IEEE Conference on Computer Vision and Pattern Recognition (CVPR)},
  year = {2014},
  organization = {IEEE},
  url = {http://www.ee.iisc.ernet.in/labs/cvl/papers/photometric_CVPR2014_paper.pdf}
}

http://giteit.udp.cl/vision-2015/prototipo-proyecyo-final.git, profe en el git se encuentra toda la informacion por que nuestro .zip era de mayor tamaño que el permitido.

Para poder realizar reconstruccion 3D con nuestro codigo primero es necesario capturar las imagenes con la Kinect, para esto hay que posicionarse en la carpeta de codigo he ir a la carpeta sacaFotos (/codigo/sacaFotos), aqui se encuentran los codigos necesarios para sacar las fotos con la kinect a traves del sensor IR, este codigo se desarrollo con openFramework (es necesario tener una fuente IR que no sea la de la Kinect por que esta agrega ruido).
Para obtener la reconstruccion 3D del objeto es necesario ir a la carpeta /codigo/opencv, aqui en la carpe src se encuetra el main de la reconstruccion es necesario que las imagenes que desea utilizar para reconstruir se encuentren en la carpta imagenes con un orden ascendiente en numeracion , es decir las fotos recibiran nombres como 0.png a 4.png, ademas en esta carpeta es necesario incorporar una imagen del ambiente (ambiente.png). Al realizar la compilacion (solo es necesario compilar con ./main ) del codigo con las correspondientes imagenes, en la carpeta src se encontrara la reconstruccion 3D que se llama export.obj, y las imagenes de la mascara, normal y mapa de profundidad.

