#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "app.h"

uint8_t cutStrOut[64];
uint8_t record_type = 0;
bool header_check = false;
bool header_record_flag = true;
bool exception_record_type = false;
bool record_terminate = false;
uint64_t record_count = 0;
uint8_t addr_length = 0;
uint8_t data_length = 0;

static Frame_Error_t frameStatus = frame_Correct;

uint8_t* cutStr(uint8_t *s, int start, int end)
{
    for (uint8_t i = start; i <= end; i++)
    {
        cutStrOut[i - start] = s[i];
    }
    cutStrOut[end - start + 1] = '\0';
    return cutStrOut;
}

uint8_t isCorrectHexFormat(uint8_t* frame){
	uint8_t i = 1;
	if(frame[0] != 'S'){
		return FRAME_ERROR;
	}
	while((frame[i] != 10) && (frame[i] != 13)){
		if(('A' <= frame[i] && frame[i] <= 'F') || ('0' <= frame[i] && frame[i] <= '9')){
			i++;
		}
		else{
			return FRAME_ERROR;
		}
	}
	return FRAME_CORRECT;
}

uint8_t checkFrameFormat(uint8_t *frame){
	if(record_terminate){
		frameStatus = frame_Error_Format;
		return frameStatus;
	}
	if(!isCorrectHexFormat(frame)){
		frameStatus = frame_Error_Format;
		return frameStatus;
	}
	uint8_t byte_count[2];
	uint8_t byte_count_check;
	uint8_t address[8];
	uint8_t data[64];
	uint8_t checksum[2];
	uint32_t sum = 0;
	strcpy(byte_count, cutStr(frame, 2, 3));
	byte_count_check = strtol(byte_count, NULL, 16);
	if(byte_count_check < 3){
		frameStatus = frame_Error_Byte_Count;
		return frameStatus;
	}
	uint8_t type = frame[1];
	uint8_t checktype = type - 48;
	if(checktype < 0 || checktype > 9 || checktype == 4){
		frameStatus = frame_Error_Record_type;
		return frameStatus;
	}
	if(checktype >= 5 && checktype <=9){
		exception_record_type = true;
	}
	if(header_record_flag == false && exception_record_type == false && checktype != record_type){
		frameStatus = frame_Error_Record_type;
		return frameStatus;
	}
	if(checktype == 0){
		if(header_record_flag == false){
			frameStatus = frame_Error_Format;
			return frameStatus;
		}
		addr_length = 4;
		strcpy(address, cutStr(frame, 4, 7));
		for(uint8_t i=0; i<strlen(address)-1; i+=2){
			sum += strtol(cutStr(address, i, i+1), NULL, 16);
		}
		if(sum != 0){
			frameStatus = frame_Error_Format;
			return frameStatus;
		}
		data_length = byte_count_check*2 - 6;
		strcpy(data, cutStr(frame, 8, 7 + byte_count_check*2 - 6));
		for(uint8_t j=0; j<strlen(data)-1; j+=2){
			sum += strtol(cutStr(data, j, j+1), NULL, 16);
		}
		sum += byte_count_check;
		sum = sum & 0xFF;
		strcpy(checksum, cutStr(frame, 8 + byte_count_check*2 - 6, 9 + byte_count_check*2 - 6));
		if((0xFF - strtol(checksum, NULL, 16)) != sum){
			frameStatus = frame_Error_Checksum;
			return frameStatus;
		}
		header_check = true;
	}

	if(checktype == 1){
		if(header_check == false){
			frameStatus = frame_Error_Format;
			return frameStatus;
		}
		if(exception_record_type == true){
			frameStatus = frame_Error_Format;
			return frameStatus;
		}
		addr_length = 4;
		strcpy(address, cutStr(frame, 4, 7));
		for(uint8_t i=0; i<strlen(address)-1; i+=2){
			sum += strtol(cutStr(address, i, i+1), NULL, 16);
		}
		data_length = byte_count_check*2 - 6;
		strcpy(data, cutStr(frame, 8, 7 + byte_count_check*2 - 6));
		for(uint8_t j=0; j<strlen(data)-1; j+=2){
			sum += strtol(cutStr(data, j, j+1), NULL, 16);
		}
		sum += byte_count_check;
		sum = sum & 0xFF;
		strcpy(checksum, cutStr(frame, 8 + byte_count_check*2 - 6, 9 + byte_count_check*2 - 6));
		if((0xFF - strtol(checksum, NULL, 16)) != sum){
			frameStatus = frame_Error_Checksum;
			return frameStatus;
		}
	}

	if(checktype == 2){
		if(header_check == false){
			frameStatus = frame_Error_Format;
			return frameStatus;
		}
		if(exception_record_type == true){
			frameStatus = frame_Error_Format;
			return frameStatus;
		}
		addr_length = 6;
		strcpy(address, cutStr(frame, 4, 9));
		for(uint8_t i=0; i<strlen(address)-1; i+=2){
			sum += strtol(cutStr(address, i, i+1), NULL, 16);
		}
		data_length = byte_count_check*2 - 8;
		strcpy(data, cutStr(frame, 10, 9 + byte_count_check*2 - 8));
		for(uint8_t j=0; j<strlen(data)-1; j+=2){
			sum += strtol(cutStr(data, j, j+1), NULL, 16);
		}
		sum += byte_count_check;
		sum = sum & 0xFF;
		strcpy(checksum, cutStr(frame, 10 + byte_count_check*2 - 8, 11 + byte_count_check*2 - 8));
		if((0xFF - strtol(checksum, NULL, 16)) != sum){
			frameStatus = frame_Error_Checksum;
			return frameStatus;
		}
	}

	if(checktype == 3){
		if(header_check == false){
			frameStatus = frame_Error_Format;
			return frameStatus;
		}
		if(exception_record_type == true){
			frameStatus = frame_Error_Format;
			return frameStatus;
		}
		addr_length = 8;
		strcpy(address, cutStr(frame, 4, 11));
		for(uint8_t i=0; i<strlen(address)-1; i+=2){
			sum += strtol(cutStr(address, i, i+1), NULL, 16);
		}
		data_length = byte_count_check*2 - 10;
		strcpy(data, cutStr(frame, 12, 11 + byte_count_check*2 - 10));
		for(uint8_t j=0; j<strlen(data)-1; j+=2){
			sum += strtol(cutStr(data, j, j+1), NULL, 16);
		}
		sum += byte_count_check;
		sum = sum & 0xFF;
		strcpy(checksum, cutStr(frame, 12 + byte_count_check*2 - 10, 13 + byte_count_check*2 - 10));
		if((0xFF - strtol(checksum, NULL, 16)) != sum){
			frameStatus = frame_Error_Checksum;
			return frameStatus;
		}
	}

	if(header_record_flag == true && checktype != 0 && checktype != 5 && checktype != 6){
		record_type = checktype;
		header_record_flag = false;
	}

	if(checktype == 5){
		addr_length = 4;
		data_length = 0;
		if(record_count > 0xFFFD){
			frameStatus = frame_Error_Record_Count;
			return frameStatus;
		}
		strcpy(address, cutStr(frame, 4, 7));
		for(uint8_t i=0; i<strlen(address)-1; i+=2){
			sum += strtol(cutStr(address, i, i+1), NULL, 16);
		}
		sum += byte_count_check;
		sum = sum & 0xFF;
		strcpy(checksum, cutStr(frame, 8, 9));
		if((0xFF - strtol(checksum, NULL, 16)) != sum){
			frameStatus = frame_Error_Checksum;
			return frameStatus;
		}
	}

	if(checktype == 6){
		addr_length = 6;
		data_length = 0;
		if(record_count < 0xFFFE){
			frameStatus = frame_Error_Record_Count;
			return frameStatus;
		}
		strcpy(address, cutStr(frame, 4, 9));
		for(uint8_t i=0; i<strlen(address)-1; i+=2){
			sum += strtol(cutStr(address, i, i+1), NULL, 16);
		}
		sum += byte_count_check;
		sum = sum & 0xFF;
		strcpy(checksum, cutStr(frame, 10, 11));
		if((0xFF - strtol(checksum, NULL, 16)) != sum){
			frameStatus = frame_Error_Checksum;
			return frameStatus;
		}
	}

	if(checktype == 7){
		addr_length = 8;
		data_length = 0;
		if(record_type != 3){
			frameStatus = frame_Error_Terminate_Type;
			return frameStatus;
		}
		strcpy(address, cutStr(frame, 4, 11));
		for(uint8_t i=0; i<strlen(address)-1; i+=2){
			sum += strtol(cutStr(address, i, i+1), NULL, 16);
		}
		sum += byte_count_check;
		sum = sum & 0xFF;
		strcpy(checksum, cutStr(frame, 12, 13));
		if((0xFF - strtol(checksum, NULL, 16)) != sum){
			frameStatus = frame_Error_Checksum;
			return frameStatus;
		}
		record_terminate = true;
	}

	if(checktype == 8){
		addr_length = 6;
		data_length = 0;
		if(record_type != 2){
			frameStatus = frame_Error_Terminate_Type;
			return frameStatus;
		}
		strcpy(address, cutStr(frame, 4, 9));
		for(uint8_t i=0; i<strlen(address)-1; i+=2){
			sum += strtol(cutStr(address, i, i+1), NULL, 16);
		}
		sum += byte_count_check;
		sum = sum & 0xFF;
		strcpy(checksum, cutStr(frame, 10, 11));
		if((0xFF - strtol(checksum, NULL, 16)) != sum){
			frameStatus = frame_Error_Checksum;
			return frameStatus;
		}
		record_terminate = true;
	}

	if(checktype == 9){
		addr_length = 4;
		data_length = 0;
		if(record_type != 1){
			frameStatus = frame_Error_Terminate_Type;
			return frameStatus;
		}
		strcpy(address, cutStr(frame, 4, 7));
		for(uint8_t i=0; i<strlen(address)-1; i+=2){
			sum += strtol(cutStr(address, i, i+1), NULL, 16);
		}
		sum += byte_count_check;
		sum = sum & 0xFF;
		strcpy(checksum, cutStr(frame, 8, 9));
		if((0xFF - strtol(checksum, NULL, 16)) != sum){
			frameStatus = frame_Error_Checksum;
			return frameStatus;
		}
		record_terminate = true;
	}

	record_count++;

	return frameStatus;
}

uint8_t findAddrLength(){
	return addr_length;
}

uint8_t findDataLength(){
	return data_length;
}

uint8_t terminationCorrect(){
	return record_terminate;
}
