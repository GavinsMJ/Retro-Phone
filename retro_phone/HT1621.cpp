#include "HT1621.h"

/**	пользовательские функции **/

//		инициализация модуля
void	HT1621::begin(uint8_t i, uint8_t j, uint8_t k){
//			установка выводов и режима работы
			SI3_line_RES=k;								//  номер вывода CS
			SI3_mass_STATUS[3]=3;						//  режим работы SI3 = mode3
			pinMode(SI3_line_RES, OUTPUT);				//  переводим вывод CS в режим выхода
			digitalWrite(SI3_line_RES, HIGH);			//  устанавливаем логическую «1» на выводе CS (не активное состояние)
			SI3_func_begin(i, j);						//  номера выводов CLK и DAT
//			отправка команд в LCD модуль
			digitalWrite(SI3_line_RES,LOW); SI3_func_WRITE_BITS(0b100000000010,12); digitalWrite(SI3_line_RES,HIGH);	//	ВКЛЮЧЕНИЕ СИСТЕМЫ
			digitalWrite(SI3_line_RES,LOW); SI3_func_WRITE_BITS(0b100001010010,12); digitalWrite(SI3_line_RES,HIGH);	//	РЕЖИМ РАБОТЫ
			clrScr();									//	чистим массив вывода buffer
			update();									//	чистим экран, выводя на него массив buffer
			digitalWrite(SI3_line_RES,LOW); SI3_func_WRITE_BITS(0b100000000110,12); digitalWrite(SI3_line_RES,HIGH);	//	ВКЛЮЧЕНИЕ LCD
}

//		очистка буфера вывода
void	HT1621::clrScr(){
			for(int i=0; i<16; i++){buffer[i]=0;}		//	чистим разряды: buffer[0] = ... = buffer[15] = 0
			bufferSimv=0;								//	чистим вывод дополнительных символов
}

//		запись строки в массив buffer с указанием начальной позиции
void	HT1621::print(char* k, uint8_t j){
uint8_t		i=0;
			while(k[i] > 0 && i < 16){					//	если не достигнут конец строки, или не превышено количество разрядов
				if(k[i]>=97 && k[i]<=122){k[i]-=32;}	//	преобразуем строчные буквы в прописные
				if(k[i]>=48 && k[i]<=57)				{buffer[i+j]=LSD_segm_BIT[k[i]-48];	}else	//	цифры
				if(k[i]>=65 && k[i]<=90)				{buffer[i+j]=LSD_segm_BIT[k[i]-55];	}else	//	буквы
				if(k[i]=='(' || k[i]=='{' || k[i]=='[')	{buffer[i+j]=LSD_segm_BIT[36];		}else	//	символ открывающая скобка
				if(k[i]==')' || k[i]=='}' || k[i]==']')	{buffer[i+j]=LSD_segm_BIT[37];		}else	//	символ закрывающая скобка
				if(k[i]==':' || k[i]==';' || k[i]=='=')	{buffer[i+j]=LSD_segm_BIT[38];		}else	//	символ равно, двоеточие, точка с запятой
				if(k[i]=='.' || k[i]==',')				{buffer[i+j]=LSD_segm_BIT[39];		}else	//	символ точка, запятая
				if(k[i]=='\'')							{buffer[i+j]=LSD_segm_BIT[40];		}else	//	символ кавычка
				if(k[i]=='"')							{buffer[i+j]=LSD_segm_BIT[41];		}else	//	символ кавычки
				if(k[i]=='?')							{buffer[i+j]=LSD_segm_BIT[42];		}else	//	символ знак вопроса
				if(k[i]=='*')							{buffer[i+j]=LSD_segm_BIT[43];		}else	//	символ градус
				if(k[i]=='-')							{buffer[i+j]=LSD_segm_BIT[44];		}else	//	символ тире
				if(k[i]=='_')							{buffer[i+j]=LSD_segm_BIT[45];		}else	//	символ нижнее тире
														{buffer[i+j]=0;						} i++;	//	всё остальное выводим как пробел
			}	symbol(16);								//	обновляем биты символов «h» в массиве buffer
}

//		запись числа в массив buffer с указанием начальной позиции (i2), системы счисления (i3) и длинны числа (i4)
void	HT1621::print(uint32_t i1, uint8_t i2, uint8_t i3, uint8_t i4){
uint32_t	i5 = i1;														//	переменная
uint8_t		i6 = 0; while(i5){i5/=i3;i6++;}									//	реальная разрядность числа
			if(i4<0 || i4>16){i4 = i6;}										//	выводимая разрядность числа
			if(i4<i6){i5=1; for(int j=0; j<i4; j++){i5*=i3;} i1%=i5;i6=i4;}	//	срезаем старшие разряды числа стоящие за пределами выводимых
			for(int i=0; i<i4-i6; i++){buffer[i2]=LSD_segm_BIT[0]; i2++;}	//	выводим нули перед числом
			for(int i=i6; i>0; i--){										//	проходим по разрядам числа
				i5=1; for(int j=1; j<i; j++){i5*=i3;}						//	делитель для выводимого разряда
				if(i2<16){buffer[i2]=LSD_segm_BIT[i1/i5]; i2++;}			//	выводим разряд
				i1%=i5;														//	удаляем разряд из числа
			}	symbol(16);													//	обновляем биты символов «h» в массиве buffer
}

//		символ вкл/выкл
void	HT1621::symbol(uint8_t i, bool j){
			if(i>=0 && i<16){if(j){bufferSimv|=_BV(i);}else{bufferSimv&=~_BV(i);}}												//	устанавливаем или сбрасываем бит в буфере символов
			for(i=0; i<16; i++){if(bufferSimv & _BV(i)){buffer[LSD_simv_BIT[i]]|=0x80;}else{buffer[LSD_simv_BIT[i]]&=0x7F;}}	//	устанавливаем или сбрасываем бит в буфере сегментов
}

//		обновление экрана
void	HT1621::update(){
			for(int i=0; i<16; i++){
			for(int j=0; j<2;  j++){
				digitalWrite(SI3_line_RES,LOW);			//  устанавливаем логический «0» на выводе CS (активное состояние)
				SI3_func_WRITE_BITS(0b1010000000000+((i*2+j)<<4)+(j?buffer[i]&0x0F:buffer[i]>>4),13);	//	пишем в регистр полубайт
				digitalWrite(SI3_line_RES,HIGH);		//  устанавливаем логическую «1» на выводе CS (не активное состояние)
			}}
}


/**	внутренние функции для работы с шиной SI3 **/
void	HT1621::SI3_func_begin		(uint8_t i, uint8_t j)							{int SI3_var_I=0; SI3_mass_STATUS[2]=1; SI3_line_CLK=i; SI3_line_DAT=j; pinMode(SI3_line_DAT, INPUT); pinMode(SI3_line_CLK, OUTPUT); digitalWrite(SI3_line_CLK, (SI3_mass_STATUS[3] & _BV(1) ? HIGH:LOW));}
void	HT1621::SI3_func_WRITE_BITS(uint32_t SI3_bytes_DATA, uint8_t SI3_byte_SUM)	{int SI3_var_I=SI3_mass_STATUS[5]? 0:SI3_byte_SUM-1; int SI3_var_J=(1000/SI3_mass_STATUS[0]-354000000/F_CPU)/2; if(SI3_var_J<0){SI3_var_J=0;} pinMode(SI3_line_DAT, OUTPUT); while(SI3_var_I>=0 && SI3_var_I<SI3_byte_SUM){ digitalWrite(SI3_line_DAT, (SI3_bytes_DATA & _BV(SI3_var_I) ? HIGH:LOW)); if(!(SI3_mass_STATUS[3] & _BV(0)) && SI3_var_J){delayMicroseconds(SI3_var_J);} digitalWrite(SI3_line_CLK, (SI3_mass_STATUS[3] & _BV(1) ? LOW:HIGH)); if(SI3_var_J){delayMicroseconds(SI3_var_J);} digitalWrite(SI3_line_CLK, (SI3_mass_STATUS[3] & _BV(1) ? HIGH:LOW)); if((SI3_mass_STATUS[3] & _BV(0)) && SI3_var_J) {delayMicroseconds(SI3_var_J);} if(SI3_mass_STATUS[5]){SI3_var_I++;}else{SI3_var_I--;}} pinMode(SI3_line_DAT, INPUT);}

