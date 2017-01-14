
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <stdio.h> 
#include <type_traits> 
#include <cmath> 
#include <time.h>
#include <fstream>

#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <omp.h>


typedef double T_DOUBLE;
typedef char T_CHAR;
typedef long T_LONG;
typedef float T_FLOAT;
typedef int T_INT;
typedef unsigned char T_BYTE;

const T_LONG BLOQUELINEA = 1024;

using namespace cv;
using namespace std;

#define norm(x, y) (fabs(x) + fabs(y)) 

//Variables globales

clock_t h_tIni, h_tFin, h_tTotal; //  Para calculo de tiempo en CPU
cudaEvent_t d_tIni, d_tFin; float d_tTotal; // Para calculo de tiempo en GPU


/*********************************************
* PARA VERIFICAR ERRORES DE CUDA QUE SE DESENCADENA DESDE EL HOST
*********************************************/

#define checkCudaErrors(err)           __checkCudaErrors (err, __FILE__, __LINE__)

inline void __checkCudaErrors(cudaError err, const char *file, const int line)
{
	if (cudaSuccess != err)
	{
		fprintf(stderr, "%s(%i) : CUDA Runtime API error %d: %s.\n",
			file, line, (int)err, cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}
}



/**********************************************
* FUNCION PARA OBTENER EL TIEMPO EN CPU
**********************************************/

double getMilisegundos(clock_t c)
{
	double tiempo = 0;
	tiempo = ((c / (double)CLOCKS_PER_SEC) * 1000);
	return tiempo;
}


/*************************************************************
* PARTE HOST
*************************************************************/

template<class T>
class h_Matriz
{

public:
	T *ptr;
	size_t row, col;
	h_Matriz(){}
	h_Matriz(size_t n)				{	inicializar(n,n);	}
	h_Matriz(size_t m, size_t n)	{	inicializar(m, n);	}
	h_Matriz(cv::Mat img)
	{
		inicializar(img.rows, img.cols);		
		for (int i = 0; i < row; i++)		
			memcpy(&(ptr[i*col]), img.ptr<T>(i, 0), col * sizeof(T));			
	}
	
	void inicializar(size_t m, size_t n)
	{
		row = m;
		col = n;
		ptr = new T[row * col];	
	}

	inline T Get(size_t r, size_t c)			{	return *(ptr + r*(col)+c);	}	
	inline void Set(size_t r, size_t c, T val)	{	*(ptr + r*(col)+c) = val;	}
	
	void Set_Matriz(h_Matriz<T> mat) 
	{
		delete ptr;
		inicializar(mat.row, mat.col);
		memcpy(&(ptr[0]), &(mat.ptr[0]), row*col * sizeof(T));
	}
	
	void Get_Matriz(h_Matriz<T> *mat)
	{
		if (mat->row == row && mat->col == col)			
			memcpy(&mat->ptr[0], &(ptr[0]), row*col * sizeof(T));
	}
	
	void h_Matriz2Mat(cv::Mat *img)
	{
		if (img->rows == row && img->cols == col)
			for (size_t i = 0; i < row; i++)
				memcpy(img->ptr<T>(i, 0), &(ptr[i*col]), col * sizeof(T));
	}
	
	void Imprimir()
	{
		for (size_t i = 0; i < row; i++)
		{
			for (size_t j = 0; j < col; j++)
				cout << ptr[i*col + j] << "\t";
			cout << endl;
		}
	}

	~h_Matriz()
	{
	}
};

// modificar el kernel para convolucion
template<class T>
void convolucion(h_Matriz<T> *kernel)
{	 	
	int r = kernel->row ;
	int c = kernel->col;
	h_Matriz<T> temp(r,c);	
	for (int k = 0; k < r; k++)
		for (int l = 0; l < c; l++)
			temp.Set(k, l, kernel->Get(r - k-1, c - l-1));
	kernel->Set_Matriz(temp);
}

// correlacion, es convolucion si el kernel es modificado para convolucion
template<class T>
void correlacion(h_Matriz<T> *img, h_Matriz<T> *imgout, h_Matriz<T> kernel)
{		
	T suma1;
	int mitad = kernel.row / 2;	
	for (int i = mitad; i < img->row-mitad;i++)
		for (int j = mitad; j < img->col - mitad; j++)
		{
			suma1 = 0;			
			for (int k = 0; k < kernel.row; k++)
				for (int l = 0; l < kernel.col; l++)					
					suma1 += img->Get(i - mitad + k, j - mitad + l)*kernel.Get(k, l);
			imgout->Set(i, j, (T)(suma1));
		}
}

// correlacion2 con dos kernel, es convolucion si los kernel es modificado para convolucion
template<class T, class T1>
void correlacion2(h_Matriz<T> *img, h_Matriz<T> *imgout, h_Matriz<T1> kernel, h_Matriz<T1> kernel2)
{
	T suma1;
	T suma2;
	T tmp;
	int mitad = kernel.row / 2;

	for (int i = mitad; i < img->row - mitad; i++)
		for (int j = mitad; j < img->col - mitad; j++){
			suma1 = 0;
			suma2 = 0;
			for (int k = 0; k < kernel.row; k++)
				for (int l = 0; l < kernel.col; l++){
					tmp = img->Get(i - mitad+k, j - mitad+l);	
					suma1 += tmp*kernel.Get(k, l);	
					suma2 += tmp*kernel2.Get(k, l);
				}
			T val = norm((T)suma1, (T)suma2);
			imgout->Set(i, j, (T)val);
		}
}

/*************************************************************
* PARTE HOST - paralelo con OpenMP
*************************************************************/
// correlacion, es convolucion si el kernel es modificado para convolucion
template<class T>
void p_correlacion(h_Matriz<T> *img, h_Matriz<T> *imgout, h_Matriz<T> kernel)
{
	T suma1;
	int mitad = kernel.row / 2;
	int i, j, k, l;
	int ir = img->row - mitad;
	int ic = img->col - mitad;
	int kr = kernel.row;
	int kc = kernel.row;
	//kernel.Imprimir();
#pragma omp parallel  for shared(kernel, img,imgout, ir, ic,kr,kc, mitad) private( i, j, k, l, suma1) 	
	for (int i = mitad; i < ir; i++)
		for (int j = mitad; j < ic; j++)
		{
			suma1 = 0;
			for (int k = 0; k < kr; k++)
				for (int l = 0; l < kc; l++)
					suma1 += img->Get(i - mitad + k, j - mitad + l)*kernel.Get(k, l);
			//std::cout << suma1<<std::endl;
			imgout->Set(i, j, (T)(suma1));
		}
}

// correlacion2 con dos kernel, es convolucion si los kernel es modificado para convolucion
template<class T, class T1>
void p_correlacion2(h_Matriz<T> *img, h_Matriz<T> *imgout, h_Matriz<T1> kernel, h_Matriz<T1> kernel2)
{
	T suma1;
	T suma2;
	T tmp;
	T_INT mitad = kernel.row / 2;
	T val;	
	T_INT ir = img->row - mitad;
	T_INT ic = img->col - mitad;
	T_INT kr = kernel.row;
	T_INT kc = kernel.row;
	T_INT i, j, k, l;
	omp_set_num_threads(8);
#pragma omp parallel  for shared(kernel, kernel2, img,imgout, ir, ic,kr,kc, mitad) private( i, j, k, l, suma1, suma2,tmp) 
	for (i = mitad; i < ir; i++){
		for (j = mitad; j < ic; j++){
			suma1 = 0;
			suma2 = 0;
			for (k = 0; k < kr; k++){
				for (l = 0; l < kc; l++){
					tmp = img->Get(i - mitad + k, j - mitad + l);
					suma1 += tmp*kernel.Get(k, l);
					suma2 += tmp*kernel2.Get(k, l);
				}
			}
			imgout->Set(i, j, (T)norm((T)suma1, (T)suma2));
		}
	}
}
/*****************************************************************
* PARTE DEVICE
*****************************************************************/

template<class T>
class d_Matriz
{

public:
	T *d_ptr;
	size_t row, col;
	d_Matriz(h_Matriz<T> mat)
	{
		inicializar(mat);
	}

	void inicializar(h_Matriz<T> mat)
	{
		row = mat.row;
		col = mat.col;
		checkCudaErrors(cudaMalloc((void**)&d_ptr, row*col * sizeof(T)));
		checkCudaErrors(cudaMemcpy(d_ptr, mat.ptr, col* row*sizeof(T), cudaMemcpyHostToDevice));		
	}

	__device__ inline T Get(size_t r, size_t c)				{ return *(d_ptr + r*(col)+c); }
	__device__ inline void Set(size_t r, size_t c, T val)	{ *(d_ptr + r*(col)+c) = val; }

	void Set_Matriz(h_Matriz<T> mat)
	{		
		checkCudaErrors(cudaFree(d_ptr));
		inicializar(mat);		
	}
	
	void Get_Matriz(h_Matriz<T> mat)
	{
		if (mat.row == row && mat.col == col)
			checkCudaErrors(cudaMemcpy(mat.ptr, d_ptr, col* row*sizeof(T), cudaMemcpyDeviceToHost));
	}	

	~d_Matriz()
	{
	}
};

// modificar el kernel para convolucion
template<class T>
__device__ void d_conv3(d_Matriz<T> *kernel)
{
	int r = kernel->row - 1;
	int c = kernel->col - 1;

	d_Matriz<T> temp(3);
	for (int k = 0; k <= r; k++)
		for (int l = 0; l <= c; l++)	
			temp.Set(k, l, kernel->Get(r-k, c-l));
	kernel->Set_Matriz(temp);
}

// correlacion, es convolucion si el kernel es modificado para convolucion
template<class T>
__global__ void d_correlacion(d_Matriz<T> img, d_Matriz<T> imgout, d_Matriz<T> kernel)
{
	T suma1;	
	
	size_t i = threadIdx.x;
	size_t j = blockIdx.x;	
	T_INT mitad = kernel.row / 2;
	while (j<(img.row - mitad)*img.col)
	{				
		suma1 = 0;
		for (T_INT k = 0; k < kernel.row; k++)
			for (T_INT l = 0; l < kernel.col; l++)
				suma1 += img.Get(i - mitad + k, j - mitad + l)*kernel.Get(k, l);
		imgout.Set(i, j, (T)(suma1));
		
		j += blockDim.x*gridDim.x;
	}
}
//__device__ void d_correlacion(d_Matriz<T> &img, d_Matriz<T> &imgout, d_Matriz<T> kernel)



// correlacion2 con dos kernel, es convolucion si los kernel es modificado para convolucion
template<class T, class T1>
__global__ void d_correlacion2(d_Matriz<T> img, d_Matriz<T> imgout, d_Matriz<T1> kernel, d_Matriz<T1> kernel2)
{
	T suma1;
	T suma2;
	T tmp;
	T_INT mitad = kernel.row / 2;
	size_t i = threadIdx.x;
	size_t j = blockIdx.x;
	
	while (j<(img.row - mitad)*img.col)
	{
		suma1 = 0;
		suma2 = 0;
		for (T_INT k = 0; k < kernel.row; k++)
			for (T_INT l = 0; l < kernel.col; l++){
				tmp = img.Get(i - mitad+k, j-mitad+l);	
				suma1 += tmp*kernel.Get(k, l);	
				suma2 += tmp*kernel2.Get(k, l);
			}
			
		imgout.Set(i, j, (T)norm((T)suma1, (T)suma2));
		j += blockDim.x*gridDim.x;
	}
}

/*****************************************************************
* OBTENER FILTRO DE ARCHIVO.KER
*****************************************************************/
template<class T>
void AbrirKernel(T_CHAR *FileOrigen, h_Matriz<T> *kernel, h_Matriz<T> *kernel2, T_INT &nroKernel)
{
	std::ifstream origen(FileOrigen);

	if (origen.fail())
		std::cerr << "Error al abrir el kernel: " << FileOrigen << std::endl;
	else
	{
		T_CHAR *bloque;
		bloque = new T_CHAR[BLOQUELINEA + 1];
		T_INT m, n;
		T_FLOAT val;
		// Leer el numero de Kernel
		origen.getline(bloque, BLOQUELINEA, '\n');
		nroKernel = atoi(bloque);		
		
		if (nroKernel == 1)// 1 solo kernel
		{
			origen.getline(bloque, BLOQUELINEA, '\n');
			m = atoi(bloque);
			origen.getline(bloque, BLOQUELINEA, '\n');
			n = atoi(bloque);
			kernel->inicializar(m, n);
			kernel2->inicializar(m, n);

			for (T_INT i = 0; i < m; i++) // llenar la matriz
				for (T_INT j = 0; j < m; j++)
				{
					origen.getline(bloque, BLOQUELINEA, '\n');
					val = atof(bloque);
					kernel->Set(i, j, val);
					kernel2->Set(i, j, val);
				}		
		}
		else // se supone que son dos filtros para una misma convolucion ejemplo sobel
		{
			// para el primer kernel
			origen.getline(bloque, BLOQUELINEA, '\n');
			m = atoi(bloque);
			origen.getline(bloque, BLOQUELINEA, '\n');
			n = atoi(bloque);
			kernel->inicializar(m, n);			

			for (T_INT i = 0; i < m; i++) // llenar la matriz
				for (T_INT j = 0; j < m; j++)
				{
					origen.getline(bloque, BLOQUELINEA, '\n');
					val = atof(bloque);
					kernel->Set(i, j, val);			
				}
			
			// para el segundo kernel
			origen.getline(bloque, BLOQUELINEA, '\n');
			m = atoi(bloque);
			origen.getline(bloque, BLOQUELINEA, '\n');
			n = atoi(bloque);
			kernel2->inicializar(m, n);

			for (T_INT i = 0; i < m; i++) // llenar la matriz
				for (T_INT j = 0; j < m; j++)
				{
					origen.getline(bloque, BLOQUELINEA, '\n');
					val = atof(bloque);
					kernel2->Set(i, j, val);
				}
		}		
	}
}



int main() // main en video
{

	//namedWindow("ventana", CV_WINDOW_AUTOSIZE);	
	//cargar el archivo de video especificado
	cv::VideoCapture cvideo("video2.mp4");	
	
	//verificar si se ha podio cargar el video
	if (!cvideo.isOpened())	return -1;
	// obtener los cuadros por segundo
	T_DOUBLE fps = cvideo.get(CV_CAP_PROP_FPS);
	T_DOUBLE nf = cvideo.get(CV_CAP_PROP_FRAME_COUNT);
	cout << "Nro de frames: " << nf<< endl;
		cout<<"Nro frames por segundos: " <<fps << endl;
	
	// calcular el tiempo de espera entre cada imagen a mostrar
	//int delay = 1000 / fps;
	T_INT delay = 1;
	h_Matriz<float> kernelx;
	h_Matriz<float> kernely;

	
	/**********************************************
	* M O D O
	**********************************************/
	int modo = 3;// CPU:1, PAR:2, GPU:3
	int filtros = 1;


	//------------------------------------------------
	// OBTENER KERNEL PARA FILTRO
	//------------------------------------------------
	
	char *modoNombre;
	modoNombre = new char[250];
	
	//strcpy(modoNombre, "sobel.ker"); // 3x3 dos filtros
	//strcpy(modoNombre, "repujado.ker");// 3x3 un filtro

	//strcpy(modoNombre, "media3.ker");
	//strcpy(modoNombre, "media5.ker");
	//strcpy(modoNombre, "media11.ker");
	//strcpy(modoNombre, "media15.ker");	
	strcpy(modoNombre, "media25.ker");

	
	AbrirKernel<T_FLOAT>(modoNombre,&kernelx,&kernely,filtros);
	
	std::cout << "Kernel: "<<modoNombre <<std::endl;
	kernelx.Imprimir();
	kernely.Imprimir();
	std::cout << "presione g para correr en gpu," << std::endl << " p para correr en cpu paralelo y " << std::endl << " c para correr en cpu secuencial...." << std::endl<<" esc para salir... o esperar que termine el video...";
	getchar();
	
	// para cambiar el kernel para obtener kernel para convolution

	convolucion(&kernelx);
	convolucion(&kernely);
	//std:: cout << "Kernel con covolucion"<<std::endl;
	//kernelx.Imprimir();
	//kernely.Imprimir();

	// Para el DEVICE
	
	d_Matriz<float> d_kernelx(kernelx);
	d_Matriz<float> d_kernely(kernely);

	//d_correlacion2<float> << <imagen.col, 1024 >> >(d_imagen, d_imagenout, d_kernelx, d_kernely);	
	//d_imagenout.Get_Matriz(imagenout);

	Mat cvimagen, cvimageng ;
	double contf = 0;

	while (contf <nf)
	{		
		cvideo >> cvimagen;	
		
		cv::cvtColor(cvimagen, cvimageng, CV_BGR2GRAY);				
		cvimageng.convertTo(cvimageng, CV_32FC1);	
		
		// en HOST		
		h_Matriz<float> imagen(cvimageng);
		h_Matriz<float> imagenout(imagen.row, imagen.col);
		//cargar el primer cuadro o imagen del video en frame
			

		switch (modo)
		{
		
			case 1:
			{
				h_tIni = clock();
				if (filtros==2)
					correlacion2(&imagen, &imagenout, kernelx, kernely);
				else
					correlacion(&imagen, &imagenout, kernelx);
				h_tFin = clock();
				strcpy(modoNombre, "Salida");
				cout << "CPU : " << getMilisegundos(h_tFin - h_tIni) << "ms por frame" << endl;
				break;
			}

			case 2:
			{
				h_tIni = clock();
				if (filtros == 2)
					p_correlacion2(&imagen, &imagenout, kernelx, kernely);
				else
					p_correlacion(&imagen, &imagenout, kernelx);
				h_tFin = clock();
				strcpy(modoNombre, "Salida");
				cout << "CPU Paralela: " << getMilisegundos(h_tFin - h_tIni) << "ms por frame" << endl;
				break;
			}

			case 3:
			{
				float d_ttemp;
				cudaEventCreate(&d_tIni);
				cudaEventCreate(&d_tFin);
				cudaEventRecord(d_tIni, 0);
				d_Matriz<float> d_imagen(imagen);
				d_Matriz<float> d_imagenout(imagenout);
				if (filtros == 2)
					d_correlacion2<float> <<<imagen.col, imagen.row / 2 >>>(d_imagen, d_imagenout, d_kernelx, d_kernely);
				else
					d_correlacion<float> <<<imagen.col, imagen.row / 2 >>>(d_imagen, d_imagenout, d_kernelx);
				d_imagenout.Get_Matriz(imagenout);				
				checkCudaErrors(cudaFree(d_imagen.d_ptr));
				checkCudaErrors(cudaFree(d_imagenout.d_ptr));

				cudaEventRecord(d_tFin, 0);
				cudaEventSynchronize(d_tFin);
				cudaEventElapsedTime(&d_ttemp, d_tIni, d_tFin);
				strcpy(modoNombre, "Salida");
				cout << "GPU: " << d_ttemp << "ms por frame" << endl;
				break;
			}
			default:
				break;

		}		
		
		imagenout.h_Matriz2Mat(&cvimageng);
		
		cvimageng.convertTo(cvimageng, CV_8UC1);
		cv::imshow(modoNombre, cvimageng);
		
		//esperar un periodo de tiempo especificado por delay 
		//si se presiona la tecla 27 (ESC) salir del loop
		uchar tec=cv::waitKey(delay);
		//cout << tec<<endl;

		if (tec == 99) modo = 1;
		if (tec == 112) modo = 2;
		if (tec == 103) modo = 3;

		if (tec == 27 ) break;
		contf++;
		delete imagen.ptr;
		delete imagenout.ptr;
	}
	cout << "Ups, se termino el video" << endl;
	cv::waitKey();
	cv::destroyWindow("ventana");

}

int mainimagen() // main en imagen
{
		// calcular el tiempo de espera entre cada imagen a mostrar
	//int delay = 1000 / fps;
	T_INT delay = 1;
	h_Matriz<float> kernelx;
	h_Matriz<float> kernely;


	/**********************************************
	* M O D O
	**********************************************/
	int modo = 3;// CPU:1, PAR:2, GPU:3
	int filtros = 1;


	//------------------------------------------------
	// OBTENER KERNEL PARA FILTRO
	//------------------------------------------------

	char *modoNombre;
	modoNombre = new char[250];

	//strcpy(modoNombre, "sobel.ker"); // 3x3 dos filtros
	//strcpy(modoNombre, "repujado.ker");// 3x3 un filtro

	//strcpy(modoNombre, "media3.ker");
	//strcpy(modoNombre, "media5.ker");
	//strcpy(modoNombre, "media11.ker");
	//strcpy(modoNombre, "media15.ker");	
	strcpy(modoNombre, "media25.ker");
	

	AbrirKernel<T_FLOAT>(modoNombre, &kernelx, &kernely, filtros);

	std::cout << "Kernel: " << modoNombre << std::endl;
	kernelx.Imprimir();
	kernely.Imprimir();

	
	// para cambiar el kernel para obtener kernel para convolution

	convolucion(&kernelx);
	convolucion(&kernely);
	//std::cout << "Kernel con covolucion" << std::endl;
	//kernelx.Imprimir();
	//kernely.Imprimir();

	

	// Para el DEVICE

	d_Matriz<float> d_kernelx(kernelx);
	d_Matriz<float> d_kernely(kernely);
		
	Mat cvimagen, cvimageng;
	double contf = 0;

	cvimagen = imread("alpaca1000.jpg", 1);
	cv::cvtColor(cvimagen, cvimageng, CV_BGR2GRAY);
	cvimageng.convertTo(cvimageng, CV_32FC1);

	// en HOST		
	h_Matriz<float> imagen(cvimageng);
	h_Matriz<float> imagenout(imagen.row, imagen.col);
	//cargar el primer cuadro o imagen del video en frame
	
	
	//---------------------------------------
	// Corrida en CPU un solo nucleo
	//---------------------------------------
	h_tIni = clock();
	if (filtros == 2)
		correlacion2(&imagen, &imagenout, kernelx, kernely);
	else
		correlacion(&imagen, &imagenout, kernelx);
	h_tFin = clock();
	strcpy(modoNombre, "Salida");
	std::cout << "CPU : " << getMilisegundos(h_tFin - h_tIni) << "ms por frame" << endl;
	
	
	//---------------------------------------
	// Corrida en CPU Paralela Utilizando OpenMP 
	//---------------------------------------
	h_tIni = clock();
	if (filtros == 2)
		p_correlacion2(&imagen, &imagenout, kernelx, kernely);
	else
		p_correlacion(&imagen, &imagenout, kernelx);
	h_tFin = clock();
	strcpy(modoNombre, "Salida");
	std::cout << "CPU Paralela: " << getMilisegundos(h_tFin - h_tIni) << "ms por frame" << endl;
	
	

	//---------------------------------------
	// Corrida en GPU , el tiempo incluye copia de archivo a memorua GPU y viceversa
	//---------------------------------------
	float d_ttemp;
	cudaEventCreate(&d_tIni);
	cudaEventCreate(&d_tFin);
	cudaEventRecord(d_tIni, 0);
	d_Matriz<float> d_imagen(imagen);
	d_Matriz<float> d_imagenout(imagenout);
	if (filtros == 2)
		d_correlacion2<float> << <imagen.col, imagen.row / 2 >> >(d_imagen, d_imagenout, d_kernelx, d_kernely);
	else
		d_correlacion<float> << <imagen.col, imagen.row / 2 >> >(d_imagen, d_imagenout, d_kernelx);
	d_imagenout.Get_Matriz(imagenout);
	checkCudaErrors(cudaFree(d_imagen.d_ptr));
	checkCudaErrors(cudaFree(d_imagenout.d_ptr));

	cudaEventRecord(d_tFin, 0);
	cudaEventSynchronize(d_tFin);
	cudaEventElapsedTime(&d_ttemp, d_tIni, d_tFin);
	strcpy(modoNombre, "Salida");
	std::cout << "GPU: " << d_ttemp << "ms por frame" << endl;
	
	
	imagenout.h_Matriz2Mat(&cvimageng);

	cvimageng.convertTo(cvimageng, CV_8UC1);
	cv::imshow(modoNombre, cvimageng);
	
	cv::waitKey();
	delete imagen.ptr;
	delete imagenout.ptr;
	std::cout << "Ups, se termino....." << endl;
	cv::destroyWindow("ventana");	
	return 0;
}



