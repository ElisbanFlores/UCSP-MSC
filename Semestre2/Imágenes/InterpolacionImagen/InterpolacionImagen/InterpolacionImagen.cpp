// InterpolacionImagen.cpp : Defines the entry point for the console application.
//--------------------------------------------------------------
// Interpolacion de imagenes: caso zoom de imagenes pequeñas
// Interpolacion vecino cercano, Bilineal y Bicubica....
// Requiere Visual Studio 2013 Comunity y OpenCv
// Autor: Elisban Flores Quenaya
// MCC - UCSP
// Arequipa, Noviembre del 2016
// -------------------------------------------------------------

#include "stdafx.h"

#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>

typedef char T_CHAR;
typedef long T_LONG;
typedef float T_FLOAT;
typedef int T_INT;
typedef unsigned char T_BYTE;

const T_LONG BLOQUELINEA = 1024;

//using namespace cv;
//using namespace std;


/*************************************************************
* Clase Matriz para guardar pixels de la imagen recuperada
*************************************************************/

template<class T>
class h_Matriz
{

public:
	T *ptr;
	size_t row, col;

	h_Matriz()						{  }
	h_Matriz(size_t n)				{ inicializar(n, n); }
	h_Matriz(size_t m, size_t n)	{ inicializar(m, n); }
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

	void Resetear()
	{
		delete ptr;
		ptr = new T[row * col];
	}

	inline T Get(size_t r, size_t c)			{ return *(ptr + r*(col)+c); }
	inline void Set(size_t r, size_t c, T val)	{ *(ptr + r*(col)+c) = val; }

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
		//for (T_INT i = row-1; i >=0; i--)
		for (T_INT i = 0; i < row; i++)
		{
			for (T_INT j = 0; j < col; j++)
				std::cout << ptr[i*col + j] << " ";// << "\t";
			std::cout << std::endl;
		}
	}

	void InvertirPorFilas()
	{
		T *temp;
		temp = new T[col];
		for (int i = 0; i < row / 2; i++)
		{
			memcpy(&(temp[0]), &(ptr[i*col]), col * sizeof(T));
			memcpy(&(ptr[i*col]), &(ptr[(row - i)*col]), col * sizeof(T));
			memcpy(&(ptr[(row - i)*col]), &(temp[0]), col * sizeof(T));
		}
	}

	~h_Matriz()
	{
		delete ptr;
	}
};
/*************************************************************
* Clase para poder trabajar con pixel a 24 bits
*************************************************************/
class ImgColor
{
private:
	T_BYTE rojo;      //intensida del rojo
	T_BYTE verde;     // intensidad del verde
	T_BYTE azul;      // intensidad del azul
	T_BYTE reservado; // sin uso
public:

public:
	inline T_INT GetRojo()      { return (T_INT)rojo; }
	inline T_INT GetVerde()     { return (T_INT)verde; }
	inline T_INT GetAzul()      { return (T_INT)azul; }
	inline T_INT GetReservado() { return (T_INT)reservado; }


	inline void SetRojo(T_INT val)      { rojo = (T_CHAR)val; }
	inline void SetVerde(T_INT val)     { verde = (T_CHAR)val; }
	inline void SetAzul(T_INT val)      { azul = (T_CHAR)val; }
	inline void SetReservado(T_INT val) { reservado = (T_CHAR)val; }

	void visualizar()
	{
		std::cout << "Rojo            : " << GetRojo() << std::endl;
		std::cout << "Verde           : " << GetVerde() << std::endl;
		std::cout << "Azul            : " << GetAzul() << std::endl;
		std::cout << "Reservado       : " << GetReservado() << std::endl;
	}

	ImgColor& operator = (const ImgColor &bmpc);
	T_INT operator == (const ImgColor &bmpc);
};

ImgColor& ImgColor::operator = (const ImgColor &bmpc)
{
	if (this != &bmpc)
	{
		this->rojo = bmpc.rojo;
		this->verde = bmpc.verde;
		this->azul = bmpc.azul;
		this->reservado = bmpc.reservado;
	}
	return *this;
}

T_INT ImgColor::operator == (const ImgColor &bmpc)
{
	if (this != &bmpc)
	{
		if (this->rojo == bmpc.rojo && this->verde == bmpc.verde&&this->azul == bmpc.azul&&this->reservado == bmpc.reservado)
			return 1;
	}
	return 0;
}

//---------------------------------------------------------
//INTERPOLACION VECINO MAS CERCANO
//---------------------------------------------------------
void VecinoCercano(cv::Mat &imagen, h_Matriz<ImgColor> &mat, T_INT escala)
{
	for (int i = 0; i < imagen.rows; i++)
		for (int j = 0; j < imagen.cols; j++)
			for (int k = 0; k < escala; k++)
				for (int l = 0; l < escala; l++)
					memcpy(&(mat.ptr[(i*escala + k)*mat.col + (j*escala + l)]), imagen.ptr<uchar>(i, j), 3);		
}

//---------------------------------------------------------
//INTERPOLACION BILINEAL
//---------------------------------------------------------
void InterpolacionBilineal(cv::Mat &imagen, h_Matriz<ImgColor> &mat, T_INT escala)
{
	float pe;
		
	// llenar la nueva imagen con los valores de la imagen original
	for (int i = 0; i < imagen.rows; i++)
		for (int j = 0; j < imagen.cols; j++)
			memcpy(&(mat.ptr[i*escala*mat.col + j*escala]), imagen.ptr<uchar>(i, j), 3);
	
	ImgColor P1, P2, P3, P4, PT;

	// realizar la interpolacion
	for (int i = 0; i < imagen.rows; i++)
		for (int j = 0; j < imagen.cols; j++)
		{			
			P1 = mat.Get((i)*escala, (j)*escala);

			// elegir los cuatro puntos, considerando los bordes externos.... previa correccion...
			if (i == imagen.rows - 1 && j == imagen.cols - 1)
				P3 = P2 = P4 = P1;
			else
			{
				if (i == imagen.rows - 1)	{
					P3 = P1; P4 = P2 = mat.Get((i)*escala, (j + 1)*escala);
				}
				else{
					if (j == imagen.cols - 1){
						P2 = P1;P4 = P3 = mat.Get((i + 1)*escala, (j)*escala);
					}
					else{
						P2 = mat.Get((i)*escala, (j + 1)*escala);
						P3 = mat.Get((i + 1)*escala, (j)*escala);
						P4 = mat.Get((i + 1)*escala, (j + 1)*escala);
					}
				}
			}
	
			//Ahora interpolar los colores
			pe = 1.0 / escala;

			for (T_FLOAT a = 0, k = 0; a < 1; a += pe, k++)
				for (T_FLOAT b = 0, l = 0; b < 1; b += pe, l++)
				{
					//canal rojo					
					PT.SetRojo((1 - a)*(1 - b)*P1.GetRojo() +
						(a)*(1 - b)*P2.GetRojo() +
						(1 - a)*(b)*P3.GetRojo() +
						(a)*(b)*P4.GetRojo());

					//canal Azul					
					PT.SetAzul((1 - a)*(1 - b)*P1.GetAzul() +
						(a)*(1 - b)*P2.GetAzul() +
						(1 - a)*(b)*P3.GetAzul() +
						(a)*(b)*P4.GetAzul());

					//canal Verde					
					PT.SetVerde((1 - a)*(1 - b)*P1.GetVerde() +
						(a)*(1 - b)*P2.GetVerde() +
						(1 - a)*(b)*P3.GetVerde() +
						(a)*(b)*P4.GetVerde());

					mat.Set((i)*escala + l, (j)*escala + k, PT);
				}
		}
}


//---------------------------------------------------------
//INTERPOLACION BICUBICA
//---------------------------------------------------------

//Funciones auxiliares
inline T_FLOAT CVal(T_FLOAT k)
{
	return k > 0.0 ? k : 0.0;
}

inline T_FLOAT PVal(T_FLOAT k)
{
	return (pow(CVal(k + 2.0), 3.0) - 4.0 * pow(CVal(k+1.0), 3.0) + 6.0 * pow(CVal(k), 3.0) - 4.0 * pow(CVal(k-1.0), 3.0))/6.0;
}

// la interpolacion en si
void InterpolacionBicubica(cv::Mat &imagen, h_Matriz<ImgColor> &mat, T_INT escala)
{
	float pe;	
	// llenar la nueva imagen con los valores de la imagen original
	for (int i = 0; i < imagen.rows; i++)
		for (int j = 0; j < imagen.cols; j++)
			memcpy(&(mat.ptr[i*escala*mat.col + j*escala]), imagen.ptr<uchar>(i, j), 3);
	
	cv::Mat imagenout((int)mat.row, (int)mat.col, CV_8UC4); // Matriz para mostrar resultados
	mat.h_Matriz2Mat(&imagenout);
	cv::imshow("Interpolacion bicubica", imagenout);

	ImgColor P[4][4]; //los 16 puntos adyacentes
	ImgColor PT;// un punto auxiliar
	T_FLOAT cr, cv, ca; // 
	T_FLOAT dx, dy; // diferenciales en x y y

	int i,j;
	int ir = imagen.rows, ic = imagen.cols;
	
	for (i = 0; i < ir; i++) // recorrer todos los pixels de la imagen  original

		for (j = 0; j < ic; j++)
		{
			// Obtener los 16 puntos considerando los bordes
			// en el origen						
			
			if (i==0){ 				
				if (j == 0){ // esquina superior izquierda
					for (int m = 1; m < 4; m++)
						for (int n = 1; n < 4; n++)
							P[m][n] = mat.Get((i + m - 1)*escala, (j + n - 1)*escala);
					P[0][0] = P[1][1];
					for (int m = 1; m < 4; m++){
						P[m][0] = P[m][1];
						P[0][m] = P[1][m];
					}
				}
				else{
					
					if (j >= ic-2){ //esquina superior derecha
						int pj = ic - j-1;						
						for (int m = 1; m < 4; m++)
							for (int n = 0; n < 2+pj; n++)
								P[m][n] = mat.Get((i + m - 1)*escala, (j + n - 1)*escala);
						for (int m = 0; m < 2+pj; m++)
							P[0][m] = P[1][m];						
						for (int m = 0; m < 4; m++){
							for (int n = 2+pj; n < 4; n++){
								P[m][n] = P[m][1+pj];
								P[m][n] = P[m][1+pj];
							}
						}
					}
					else{ // borde superior de la imagen
						for (int m = 1; m < 4; m++)
							for (int n = 0; n < 4; n++)
								P[m][n] = mat.Get((i + m - 1)*escala, (j + n - 1)*escala);
						for (int m = 0; m < 4; m++){
							P[0][m] = P[1][m];									
						}
					}
				}
			}
			else{
				if (i >= ir-2){
					int pi=ir-i-1;					
					if (j == 0){ // esquina inferior izquierda
						for (int m = 0; m < 2+pi; m++)
							for (int n = 1; n < 4; n++)
								P[m][n] = mat.Get((i + m - 1)*escala, (j + n - 1)*escala);
						for (int m = 0; m < 2+pi; m++)
							P[m][0] = P[m][1];

						for (int m = 2 + pi; m < 4; m++)
							for (int n = 0; n < 4; n++)
								P[m][n] = P[1 + pi][n];						
					}
					else{
						if (j >= ic-2){		// esquina inferior derecha					
							
							int pj = ic - j - 1;
							
							for (int m = 0; m < 2+pi; m++)
								for (int n = 0; n < 2+pj; n++)
									P[m][n] = mat.Get((i + m - 1)*escala, (j + n - 1)*escala);
							
							for (int m = 2 + pi; m < 4; m++)
								for (int n = 2+pj; n < 4; n++)
									P[m][n] = P[1 + pi][1+pj];
							
						}
						else{ // borde inferior
							for (int m = 0; m < 2+pi; m++)
								for (int n = 0; n < 4; n++)
									P[m][n] = mat.Get((i + m - 1)*escala, (j + n - 1)*escala);
							for (int m = 2+pi; m < 4; m++)
								for (int n = 0; n < 4; n++)
									P[m][n] = P[1+pi][n];
						}
					}
				}
				else{

					if (j == 0){ // borde izquierdo
						for (int m = 0; m < 4; m++)
							for (int n = 1; n < 4; n++)
								P[m][n] = mat.Get((i + m - 1)*escala, (j + n - 1)*escala);
						for (int m = 0; m < 4; m++){
							P[m][0] = P[m][1];							
						}
					}
					else{
						
						if (j >= ic-2){	// borde derecho			
							int pj = ic - j-1;							
							for (int m = 0; m < 4; m++)
								for (int n = 0; n < 2+pj; n++)
									P[m][n] = mat.Get((i + m - 1)*escala, (j + n - 1)*escala);

							for (int m = 0; m < 4; m++)
								for (int n = 2+pj; n < 4;n++)
									P[m][n] = P[m][1+pj];							
						}
						else{ // interior de la imagen
							for (int m = 0; m < 4; m++)
								for (int n = 0; n < 4; n++)
									P[m][n] = mat.Get((i + m - 1)*escala, (j + n - 1)*escala);
						}						
					}
				}
			}


			// Ahora interpolar los valores de color
			pe = 1.0 / escala;
			// rellenar espacio vacio

			for (T_FLOAT a = 0, k=0; a < 1; a += pe,k++)	{
				for (T_FLOAT b = 0, l=0; b < 1; b += pe,l++)	{
					cr = cv = ca = 0;			
					for (T_INT  m = 0; m < 4; m++){
						for (T_INT  n = 0; n < 4; n++){
							dx = PVal(a - T_FLOAT(m - 1));
							dy = PVal(b - T_FLOAT(n - 1));
							cr += dx*dy*P[m][n].GetRojo();
							cv += dx*dy*P[m][n].GetVerde();
							ca += dx*dy*P[m][n].GetAzul();
						}
					}					
					PT.SetRojo(cr);
					PT.SetVerde(cv);
					PT.SetAzul(ca);
					mat.Set((i)*escala + k, (j)*escala + l, PT);					
				}
			}
		}
}


int main()
{	
	float pe, pei;

	cv::Mat imagen;	// Matriz con la imagen original
	
	// Abrir la Imagen
	//imagen = cv::imread("tigre.bmp", 1);
	//imagen = cv::imread("caballito.bmp", 1);
	imagen = cv::imread("alpaca64.jpg", 1);
	//imagen = cv::imread("bellezadelcielo64.bmp", 1);
	//imagen = cv::imread("lenaOriginal24bitx64.bmp", 1);
	
	std::cout << "Imagen Original" << std::endl;
	cv::imshow("Imagen Original", imagen);
	//cv::waitKey();
	
	//zoom de la imagen
	int escala = 10;	// escala de la imagen
	
	h_Matriz<ImgColor> mat(imagen.rows*escala, imagen.cols*escala); // Matriz para hacer calculos
	cv::Mat imagenout((int)mat.row, (int)mat.col, CV_8UC4); // Matriz para mostrar resultados
	
	//Vecino mas cercano
	std::cout << "Interpolacion vecino mas cercano"<<std::endl;
	VecinoCercano(imagen, mat, escala);	
	mat.h_Matriz2Mat(&imagenout);
	cv::imshow("Interpolacion vecino mas cercano", imagenout);
	cv::imwrite("SalidaVecino.bmp", imagenout);
	//cv::waitKey();
		
	// Interpolacion Bilineal
	std::cout << "Interpolacion bilineal" << std::endl;
	InterpolacionBilineal(imagen, mat, escala);			
	mat.h_Matriz2Mat(&imagenout);	
	cv::imshow("Interpolacion bilineal", imagenout);
	cv::imwrite("SalidaInter_Bilineal.bmp", imagenout);
	//cv:cvWaitKey();
	
	// Interpolacion Bicubica
	std::cout << "Interpolacion bicubica" << std::endl;
	InterpolacionBicubica(imagen, mat, escala);	
	mat.h_Matriz2Mat(&imagenout);	
	cv::imshow("Interpolacion bicubica", imagenout);
	cv::imwrite("SalidaInter_Bicubica.bmp", imagenout);
	std::cout << "Presione cualquir tecla para salir" << std::endl;
	cv::waitKey();	
	
	return 0;
}

