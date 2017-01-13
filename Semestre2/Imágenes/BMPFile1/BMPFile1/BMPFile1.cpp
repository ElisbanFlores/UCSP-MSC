// BMPFile1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <string>
#include <cstring>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <time.h>
#include <cstdlib>


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
				std::cout << ptr[i*col + j]<<" ";// << "\t";
			std::cout << std::endl;
		}
	}

	void InvertirPorFilas()
	{
		T *temp;
		temp = new T[col];
		for (int i = 0; i < row/2; i++)
		{
			memcpy(&(temp[0]), &(ptr[i*col]), col * sizeof(T));
			memcpy(&(ptr[i*col]), &(ptr[(row-i)*col]), col * sizeof(T));
			memcpy(&(ptr[(row-i)*col]), &(temp[0]), col * sizeof(T));
		}
	}

	~h_Matriz()
	{
	}
};

class BMPCabeceraArchivo
{
private:
	T_BYTE tipoArchivo[2];     // 'BM'
	T_BYTE tamanhoArchivo[4];  // Tamaño del archivo en bytes
	T_BYTE reservado[4];       // libres  
	T_BYTE offsetDatos[4];     // File offset to rastes Data

public:
	inline T_CHAR *GetTipoArchivo() { T_CHAR *tipo; tipo=new T_CHAR[3]; tipo[0] = tipoArchivo[0]; tipo[1] = tipoArchivo[1]; tipo[2] = '\0'; return tipo; }
	inline T_INT GetTamanhoArchivo(){ T_INT demoi = 0; memcpy(&demoi, tamanhoArchivo, sizeof(tamanhoArchivo)); return demoi; }
	inline T_INT GetReservado()     { T_INT demoi = 0; memcpy(&demoi, reservado, sizeof(reservado)); return demoi; }
	inline T_INT GetOffsetDatos()   { T_INT demoi = 0; memcpy(&demoi, offsetDatos, sizeof(offsetDatos)); return demoi; }	
	
	inline void SetTipoArchivo(T_CHAR *val) { tipoArchivo[0] = val[0]; tipoArchivo[1] = val[1]; }
	inline void SetTamanhoArchivo(T_INT val){ memcpy(tamanhoArchivo, &val, sizeof(tamanhoArchivo)); }
	inline void SetReservado(T_INT val)     { memcpy(reservado, &val, sizeof(reservado)); }
	inline void SetOffsetDatos(T_INT val)   { memcpy(offsetDatos, &val, sizeof(offsetDatos));}
	
	void visualizar()
	{		
		std::cout << "Tipo de Archivo  : " << GetTipoArchivo()<< std::endl;		
		std::cout << "Tamaño de Archivo: " << GetTamanhoArchivo() <<std::endl;		
		std::cout << "Reservado        : " << GetReservado() << std::endl;		
		std::cout << "Offset de Datos  : " << GetOffsetDatos() << std::endl;
	}
};


class BMPCabeceraInfo
{
private:
	T_BYTE tamanho[4];           // Tamaño del Info de la cabecera 40
	T_BYTE ancho[4];             // Ancho del bitmap
	T_BYTE alto[4];              // Alto del bitmap
	T_BYTE planes[2];            // numero de planes = 1
	T_BYTE nroBitsPixel[2];      // Bits por pixel
	T_BYTE compresion[4];        // Tipo de compresion
	T_BYTE tamanhoImagen[4];     // Tamaño de la imagen; caso de comprimida
	T_BYTE XpixelPorM[4];        // Resolucion horizontal pixels por metro
	T_BYTE YpixelPorM[4];        // Resolucion vertical pixels por metro
	T_BYTE coloresUsados[4];     // Numero de colores usados
	T_BYTE coloresImportantes[4];// Numero de colores importantes 0 = todos

public:
	inline T_INT GetTamanho()           { T_INT demoi = 0; memcpy(&demoi, tamanho, sizeof(tamanho)); return demoi; }
	inline T_INT GetAncho()             { T_INT demoi = 0; memcpy(&demoi, ancho, sizeof(ancho)); return demoi; }
	inline T_INT GetAlto()              { T_INT demoi = 0; memcpy(&demoi, alto, sizeof(alto)); return demoi; }
	inline T_INT GetPlanes()            { T_INT demoi = 0; memcpy(&demoi, planes, sizeof(planes)); return demoi; }
	inline T_INT GetNroBitsPixel()      { T_INT demoi = 0; memcpy(&demoi, nroBitsPixel, sizeof(nroBitsPixel)); return demoi; }
	inline T_INT GetCompresion()        { T_INT demoi = 0; memcpy(&demoi, compresion, sizeof(compresion)); return demoi; }
	inline T_INT GetTamanhoImagen()     { T_INT demoi = 0; memcpy(&demoi, tamanhoImagen, sizeof(tamanhoImagen)); return demoi; }
	inline T_INT GetXPixelPorM()        { T_INT demoi = 0; memcpy(&demoi, XpixelPorM, sizeof(XpixelPorM)); return demoi; }
	inline T_INT GetYPixelPorM()        { T_INT demoi = 0; memcpy(&demoi, YpixelPorM, sizeof(YpixelPorM)); return demoi; }
	inline T_INT GetColoresUsados()     { T_INT demoi = 0; memcpy(&demoi, coloresUsados, sizeof(coloresUsados)); return demoi; }
	inline T_INT GetColoresImportantes(){ T_INT demoi = 0; memcpy(&demoi, coloresImportantes, sizeof(coloresImportantes)); return demoi; }
	
	
	inline void SetTamanho(T_INT val)           { memcpy(tamanho, &val, sizeof(tamanho));}
	inline void SetAncho(T_INT val)             { memcpy(ancho, &val, sizeof(ancho)); }
	inline void SetAlto(T_INT val)              { memcpy(alto, &val, sizeof(alto)); }
	inline void SetPlanes(T_INT val)            { memcpy(planes, &val, sizeof(planes));}
	inline void SetNroBitsPixel(T_INT val)      { memcpy(nroBitsPixel, &val, sizeof(nroBitsPixel));}
	inline void SetCompresion(T_INT val)        { memcpy(compresion, &val, sizeof(compresion)); }
	inline void SetTamanhoImagen(T_INT val)     { memcpy(tamanhoImagen, &val, sizeof(tamanhoImagen));}
	inline void SetXPixelPorM(T_INT val)        { memcpy(XpixelPorM, &val, sizeof(XpixelPorM)); }
	inline void SetYPixelPorM(T_INT val)        { memcpy(YpixelPorM, &val, sizeof(YpixelPorM)); }
	inline void SetColoresUsados(T_INT val)     { memcpy(coloresUsados, &val, sizeof(coloresUsados));}
	inline void SetColoresImportantes(T_INT val){ memcpy(coloresImportantes, &val, sizeof(coloresImportantes));}
	
	void visualizar()
	{
		std::cout << "Tamaño             : " << GetTamanho() << std::endl;
		std::cout << "Ancho              : " << GetAncho() << std::endl;
		std::cout << "Alto               : " << GetAlto() << std::endl;
		std::cout << "Planes             : " << GetPlanes() << std::endl;
		std::cout << "Nro Bits por Pixel : " << GetNroBitsPixel() << std::endl;
		std::cout << "Compresion         : " << GetCompresion() << std::endl;
		std::cout << "Tamaño Imagen      : " << GetTamanhoImagen() << std::endl;
		std::cout << "X pixel por M      : " << GetXPixelPorM() << std::endl;
		std::cout << "Y pixel por M      : " << GetYPixelPorM() << std::endl;
		std::cout << "Colores usados     : " << GetColoresUsados() << std::endl;
		std::cout << "Colores importantes: " << GetColoresImportantes() << std::endl;		
	}
};

class BMPColor
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

	BMPColor& operator = (const BMPColor &bmpc);
	T_INT operator == (const BMPColor &bmpc);
};

BMPColor& BMPColor::operator = (const BMPColor &bmpc)
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

T_INT BMPColor::operator == (const BMPColor &bmpc)
{
	if (this != &bmpc)
	{
		if (this->rojo == bmpc.rojo && this->verde == bmpc.verde&&this->azul == bmpc.azul&&this->reservado == bmpc.reservado)
			return 1;
	}
	return 0;
}

T_INT LeerBit(T_BYTE valor, T_INT pos)
{
	return ((valor & (int)pow(2, pos))>0 ? 1 : 0);
}

T_INT Leer4Bit(T_BYTE valor, T_INT pos)
{	
	return ((valor>>4*pos) & 15);
}

// PARA EL ARCHIVO EN FORMATO BMP

BMPCabeceraArchivo bmpCA;
BMPCabeceraInfo bmpCI;
BMPColor *bmpPaleta;
h_Matriz<BMPColor> mat;
h_Matriz<T_FLOAT> histogramaR;
h_Matriz<T_FLOAT> histogramaG;
h_Matriz<T_FLOAT> histogramaB;


void LeerPaleta(T_CHAR *FileOrigen,T_INT nc)
{	
	std::ifstream origen(FileOrigen, std::ios::binary);
	if (origen.fail())
		std::cerr << "Error al abrir el archivo: " << FileOrigen << std::endl;
	else
	{	
		origen.seekg(54); // hasta donde esta la informacion de la paleta;		
		// Leer la tabla de color			
		bmpPaleta = new BMPColor[nc]; 		
		for (int i = 0; i < nc; i++)
		{
			origen.read((char *)&bmpPaleta[i], sizeof(BMPColor));
			//bmpPaleta[i].visualizar();
		}
	}
}

// Retorna el indice en base al color de la paleta
T_INT ObtenerIndicePaleta(BMPColor color, T_INT tp)
{
	for (T_INT i = 0; i < tp; i++)
		if (bmpPaleta[i] == color)
			return i;
	return -1;
}

void LeerBMP(T_CHAR *FileOrigen)
{
	std::ifstream origen(FileOrigen, std::ios::binary);	

	if (origen.fail())
		std::cerr << "Error al abrir el archivo: " << FileOrigen << std::endl;
	else
	{			
		//Leer cabecera de Archivo
		origen.read((char *)&bmpCA,sizeof(BMPCabeceraArchivo));		
		if (strcmp(bmpCA.GetTipoArchivo(), "BM") != 0)
		{
			std::cout << "No es un Archivo BMP.... : " << FileOrigen << std::endl;
			getchar();
			exit;
		}
		bmpCA.visualizar();
		


		//Leer Info de la cabecera
		origen.read((char *)&bmpCI, sizeof(BMPCabeceraInfo));
		bmpCI.visualizar();	

		mat.inicializar(bmpCI.GetAlto(), bmpCI.GetAncho());

		std::cout << "Total tamaño de la cabecera: " << sizeof(BMPCabeceraArchivo) + sizeof(BMPCabeceraInfo)<<" bytes"<<std::endl;

		T_BYTE dato;

		switch (bmpCI.GetNroBitsPixel())
		{
			case 1:
			{
				T_INT nc = 2;
				LeerPaleta(FileOrigen, nc);					
				origen.seekg(bmpCA.GetOffsetDatos()); // hasta donde esta el contenido del archivo;

				for (int i = 0; i < mat.col*mat.row;)
				{
					if (i % 8 == 0)	origen.read((char *)&dato, sizeof(T_BYTE));
					for (T_INT j = 7; j >= 0; j--, i++)					
						mat.ptr[i] = bmpPaleta[LeerBit(dato, j)];
				}
			
				break;
			} 
			case 4:
			{	
				// Leer la tabla de color			
				T_INT nc = 16;
				LeerPaleta(FileOrigen, nc);
				origen.seekg(bmpCA.GetOffsetDatos()); // hasta donde esta el contenido del archivo;
				for (int i = 0; i < (mat.col*mat.row) ; i=i+2)
				{			
					origen.read((char *)&dato, sizeof(T_BYTE));
					mat.ptr[i] = bmpPaleta[Leer4Bit(dato, 1)];
					mat.ptr[i+1] = bmpPaleta[Leer4Bit(dato, 0)];
				}						
				break;
			}
			case 8:
			{
				// Leer la tabla de color			
				T_INT nc = 256;
				LeerPaleta(FileOrigen, nc);		
				origen.seekg(bmpCA.GetOffsetDatos()); // hasta donde esta el contenido del archivo;
				for (int i = 0; i < mat.col*mat.row;i++)
				{
					origen.read((char *)&dato, sizeof(T_BYTE));				
					mat.ptr[i] = bmpPaleta[(int)(dato)];
				}			
				break;		
			}
			case 24:
			{
				origen.seekg(bmpCA.GetOffsetDatos()); // hasta donde esta el contenido del archivo;
				T_BYTE dato3[3];			
				for (int i = 0; i < mat.col*mat.row; i++)
				{
					origen.read((char *)&dato3, sizeof(T_BYTE)*3);				
					mat.ptr[i].SetRojo(dato3[0]) ;
					mat.ptr[i].SetVerde(dato3[1]);
					mat.ptr[i].SetAzul(dato3[2]);
					mat.ptr[i].SetReservado(0) ;
				}
				break;
			}
			default:
				break;
		}
		
		mat.InvertirPorFilas();
		origen.close();
	}
}

void GuardarBMP(T_CHAR *FileDestino)
{
	std::ofstream destino(FileDestino, std::ios::binary);

	if (destino.fail())
		std::cerr << "Error al abrir el archivo: " << FileDestino << std::endl;
	else
	{
		mat.InvertirPorFilas();
		
		if (bmpCI.GetNroBitsPixel() == 24) // convertir imagenes a 24 bists de versiones recientes a una antigua....
		{
			bmpCA.SetOffsetDatos(54);
			bmpCA.SetTamanhoArchivo(54 + bmpCI.GetTamanhoImagen());
			bmpCI.SetTamanho(40);
		}
		//Escribir cabecera de Archivo
		destino.write((char *)&bmpCA, sizeof(BMPCabeceraArchivo));
		bmpCA.visualizar();

		//Guardar Info de la cabecera
		destino.write((char *)&bmpCI, sizeof(BMPCabeceraInfo));
		bmpCI.visualizar();
		
		
		T_BYTE dato;

		switch (bmpCI.GetNroBitsPixel())
		{
			case 1:
			{
				T_INT nc = 2;
			
				// gruardarPaleta
				destino.write((char *)&bmpPaleta[0], nc*sizeof(BMPColor));			

				for (int i = 0; i < mat.col*mat.row;)
				{
					dato = 0;
					for (T_INT j = 7; j >= 0; j--, i++)
						dato = mat.ptr[i].GetRojo()==255? dato + (T_BYTE)pow(2,j):dato;												
					destino.write((char *)&dato, sizeof(T_BYTE));				
				}
				break;
			}
			case 4:
			{						
				T_INT nc = 16;
				// gruardarPaleta
				destino.write((char *)&bmpPaleta[0], nc*sizeof(BMPColor));

				for (int i = 0; i < (mat.col*mat.row); i = i + 2)
				{				
					dato=(T_BYTE)ObtenerIndicePaleta(mat.ptr[i], nc);
					dato = dato << 4;
					dato += (T_BYTE)ObtenerIndicePaleta(mat.ptr[i+1], nc);				
					destino.write((char *)&dato, sizeof(T_BYTE));
				}
				break;
			}
			case 8:
			{
				// Leer la tabla de color			
				T_INT nc = 256;
				// gruardarPaleta
				
				// para cambiar de paleta generando paleta para escala de grises
				/*BMPColor *bmpPaleta2;
				bmpPaleta2 = new BMPColor[nc];
				for (T_INT i = 0; i < nc; i++)
				{
					bmpPaleta2[i].SetRojo(i);
					bmpPaleta2[i].SetVerde(i);
					bmpPaleta2[i].SetAzul(i);
					bmpPaleta2[i].SetReservado(0);
				}*/				
				destino.write((char *)&bmpPaleta[0], nc*sizeof(BMPColor));

				for (int i = 0; i < mat.col*mat.row; i++)
				{
					dato = (T_BYTE)ObtenerIndicePaleta(mat.ptr[i], nc);
					destino.write((char *)&dato, sizeof(T_BYTE));				
				}
				break;
			}
			case 24:
			{			
				T_BYTE dato3[3];
				for (int i = 0; i < mat.col*mat.row; i++)
				{
					dato3[0]=mat.ptr[i].GetRojo();
					dato3[1]=mat.ptr[i].GetVerde();
					dato3[2]=mat.ptr[i].GetAzul();				
					destino.write((char *)&dato3, sizeof(T_BYTE) * 3);				
				}
				break;
			}
			default:
				break;
		}	
		
		destino.close();
	}
}


T_INT Redondeo(T_FLOAT v)
{
	return (T_INT)((v - floor(v)) < (ceil(v) - v) ? floor(v) : ceil(v));
}


void GenerarHistograma() //
{
	T_INT tp = bmpCI.GetNroBitsPixel();

	switch (tp)
	{
	case 1: case 4: case 8:
	{
		tp = (T_INT)pow(2, tp);
		histogramaR.inicializar(tp, 4);// 0:conteo, 1:probabilidad, 2:ecualizacion 3: ecualizacion normalizada
		for (T_INT i = 0; i < tp; i++)
		{
			histogramaR.Set(i, 0, 0);	histogramaR.Set(i, 1, 0);
		}
		T_INT ind;
		T_FLOAT tam = mat.col*mat.row;
		for (T_INT i = 0; i < tam; i++)
		{
			ind = ObtenerIndicePaleta(mat.ptr[i], tp);
			histogramaR.Set(ind, 0, histogramaR.Get(ind, 0) + 1);
		}

		// generar las probabilidades
		for (T_INT i = 0; i < tp; i++)
			histogramaR.Set(i, 1, histogramaR.Get(i, 0) / tam);

		//transformacion de ecualizacion
		histogramaR.Set(0, 2, (tp - 1)*histogramaR.Get(0, 1));
		T_FLOAT v;
		v = histogramaR.Get(0, 2);
		histogramaR.Set(0, 3, Redondeo(v));
		for (T_INT i = 1; i < tp; i++)
		{
			histogramaR.Set(i, 2, histogramaR.Get(i - 1, 2) + (tp - 1)*histogramaR.Get(i, 1));
			v = histogramaR.Get(i, 2);
			histogramaR.Set(i, 3, Redondeo(v));
		}
		histogramaR.Imprimir();
		break;
	}

	case 24:
	{
		for (int c = 0; c < 3; c++)//para cada canal
		{
			// 0:conteo, 1:probabilidad, 2:ecualizacion 3: ecualizacion normalizada
			tp = 256;
			histogramaR.inicializar(tp, 4); // Histograma para canal Rojo
			histogramaG.inicializar(tp, 4); // Histograma para canal Verde
			histogramaB.inicializar(tp, 4); // Histograma para canal Azul


			for (T_INT i = 0; i < tp; i++)
			{
				histogramaR.Set(i, 0, 0);	histogramaR.Set(i, 1, 0);
				histogramaG.Set(i, 0, 0);	histogramaG.Set(i, 1, 0);
				histogramaB.Set(i, 0, 0);	histogramaB.Set(i, 1, 0);
			}

			T_INT ind;
			T_FLOAT tam = mat.col*mat.row;
			for (T_INT i = 0; i < tam; i++)
			{
				//ind = ObtenerIndicePaleta(mat.ptr[i], tp);
				ind = mat.ptr[i].GetRojo();
				histogramaR.Set(ind, 0, histogramaR.Get(ind, 0) + 1);
				ind = mat.ptr[i].GetVerde();
				histogramaG.Set(ind, 0, histogramaG.Get(ind, 0) + 1);
				ind = mat.ptr[i].GetAzul();
				histogramaB.Set(ind, 0, histogramaB.Get(ind, 0) + 1);
			}



			// generar las probabilidades
			for (T_INT i = 0; i < tp; i++)
			{
				histogramaR.Set(i, 1, histogramaR.Get(i, 0) / tam);
				histogramaG.Set(i, 1, histogramaG.Get(i, 0) / tam);
				histogramaB.Set(i, 1, histogramaB.Get(i, 0) / tam);
			}
			//transformacion de ecualizacion
			histogramaR.Set(0, 2, (tp - 1)*histogramaR.Get(0, 1));
			histogramaG.Set(0, 2, (tp - 1)*histogramaG.Get(0, 1));
			histogramaB.Set(0, 2, (tp - 1)*histogramaB.Get(0, 1));


			T_FLOAT v;
			v = histogramaR.Get(0, 2);
			histogramaR.Set(0, 3, Redondeo(v));
			v = histogramaG.Get(0, 2);
			histogramaG.Set(0, 3, Redondeo(v));
			v = histogramaB.Get(0, 2);
			histogramaB.Set(0, 3, Redondeo(v));

			for (T_INT i = 1; i < tp; i++)
			{
				histogramaR.Set(i, 2, histogramaR.Get(i - 1, 2) + (tp - 1)*histogramaR.Get(i, 1));
				v = histogramaR.Get(i, 2); histogramaR.Set(i, 3, Redondeo(v));

				histogramaG.Set(i, 2, histogramaG.Get(i - 1, 2) + (tp - 1)*histogramaG.Get(i, 1));
				v = histogramaG.Get(i, 2);	histogramaG.Set(i, 3, Redondeo(v));

				histogramaB.Set(i, 2, histogramaB.Get(i - 1, 2) + (tp - 1)*histogramaB.Get(i, 1));
				v = histogramaB.Get(i, 2);	histogramaB.Set(i, 3, Redondeo(v));
			}

			//histograma.Imprimir();

		}
		break;
	}
	default:
		break;

	}
}


//Ecualizacion de histograma
void Ecualizar()
{
	T_INT tp = bmpCI.GetNroBitsPixel();

	switch (tp)
	{
	case 1: case 4: case 8:
	{
		tp = (T_INT)pow(2, tp);

		T_INT ind;
		T_FLOAT tam = mat.col*mat.row;

		std::cout << "entre......: " << tp << " tam: " << tam << std::endl;

		for (T_INT i = 0; i < tam; i++)
		{
			ind = ObtenerIndicePaleta(mat.ptr[i], tp);
			mat.ptr[i] = bmpPaleta[(T_INT)histogramaR.Get(ind, 3)];
		}
		break;
	}
	case 24:
	{

		tp = (T_INT)pow(2, tp);

		T_INT ind;
		T_FLOAT tam = mat.col*mat.row;

		for (T_INT i = 0; i < tam; i++)
		{
			ind = mat.ptr[i].GetRojo();
			mat.ptr[i].SetRojo((T_INT)histogramaR.Get(ind, 3));
			ind = mat.ptr[i].GetVerde();
			mat.ptr[i].SetVerde((T_INT)histogramaG.Get(ind, 3));
			ind = mat.ptr[i].GetAzul();
			mat.ptr[i].SetAzul((T_INT)histogramaB.Get(ind, 3));
		}


		break;
	}

	default:
		break;
	}
}



int main()
{	
	//LeerBMP("lenaOriginal.png");
	//LeerBMP("lenaOriginalMono.bmp");	
	//LeerBMP("bellezadelcielo.bmp");	
	//LeerBMP("salida.bmp");
	//LeerBMP("lenaOriginal16colores.bmp");
	//LeerBMP("lenaOriginal16coloresx64.bmp");	
	LeerBMP("lenaOriginalgrises.bmp");
	//LeerBMP("lenaOriginal256Coloresx64.bmp");	
	//LeerBMP("lenaOriginal256Colores.bmp");
	//LeerBMP("lenaOriginal24bits.bmp");
	//LeerBMP("lenaOriginal24bitx64.bmp");
	
	
	GenerarHistograma();		
	Ecualizar();		
	GuardarBMP("salida.bmp");

	std::cout << "Archivo generado en salida.bmp "<< std::endl;
	getchar();
	return 0;
}


int main2()
{
	//LeerBMP("lenaOriginal.png");
	//LeerBMP("lenaOriginalMono.bmp");	
	LeerBMP("bellezadelcielo.bmp");	
	//LeerBMP("salida.bmp");
	//LeerBMP("lenaOriginal16colores.bmp");
	//LeerBMP("lenaOriginal16coloresx64.bmp");	
	//LeerBMP("lenaOriginalgrises.bmp");
	//LeerBMP("lenaOriginal256Coloresx64.bmp");	
	//LeerBMP("lenaOriginal256Colores.bmp");
	//LeerBMP("lenaOriginal24bits.bmp");
	//LeerBMP("lenaOriginal24bitx64.bmp");

	/*std::cout<<Leer4Bit('a', 0);
	std::cout << Leer4Bit('a', 1);*/

	GenerarHistograma();
	
	cv::Mat imagen(bmpCI.GetAlto(), bmpCI.GetAncho(), CV_8UC4);//CV_32FC1,CV_8UC1
	mat.h_Matriz2Mat(&imagen);
	cv::imshow("Original", imagen);	
	cv::waitKey();

	Ecualizar();
	
	mat.h_Matriz2Mat(&imagen);
	cv::imshow("Ecualizado", imagen);
	cv::waitKey();

	GuardarBMP("salida.bmp");
	//std::cout << "Archivo generado en salida.bmp " << std::end;
	getchar();
	return 0;
}

