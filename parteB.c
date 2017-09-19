#include <fcntl.h> 
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

int main(){

	printf("Empezando... \n");
	
	int mouse_file = open( "/dev/input/mice", O_RDONLY );
	if (mouse_file == -1) {
		printf("Error en el archivo del raton \n");
		return 1;
	}

	int key_file = open( "/dev/parteA", O_WRONLY );
	if (key_file == -1) {
		printf("Error en el archivo parteA \n");
		return 2;
	}

	do
	{
		char m_buff[3];
		
		read(mouse_file, m_buff, 3);
		
		switch(m_buff[0]){
		case 41:{
			printf("izquierdo \n");
			write(key_file, "1", 1);
		} break;   //left             00001
		case 44:{
			printf("central \n");
			write(key_file, "2", 1);
		} break;   //middle           00100
		case 42:{
			printf("derecho \n");
			write(key_file, "3", 1);
		} break;   //rigth            00010
		case 43:{
			printf("derecho + izquierdo \n");
			write(key_file, "13", 2);
		} break;   //rigth + left     00011
		case 45:{
			printf("central + izquierdo \n");
			write(key_file, "12", 2);
		} break;   //middle + left     00101
		case 46:{
			printf("derecho + central \n");
			write(key_file, "23", 2);
		} break;   //rigth + middle     00110
		case 47:{
			printf("todos \n");
			write(key_file, "123", 3);
		} break;   //all     00111
		//default : printf("valor %d \n", m_buff[0]);
		}
		
	}
	while(1);	
	
	return 0;
}
