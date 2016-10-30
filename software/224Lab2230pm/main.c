/*
 * "Hello World" example.
 *
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example
 * designs. It runs with or without the MicroC/OS-II RTOS and requires a STDOUT
 * device in your system's hardware.
 * The memory footprint of this hosted application is ~69 kbytes by default
 * using the standard reference design.
 *
 * For a reduced footprint version of this template, and an explanation of how
 * to reduce the memory footprint for a given application, see the
 * "small_hello_world" template.
 *
 */

#include <stdio.h>
#include "basic_io.h"
#include "LCD.h"
#include "SD_Card.h"
#include "fat.h"
#include "wm8731.h"
#include <math.h>
#include "drivers/inc/altera_avalon_pio_regs.h"

data_file df;
int chain[3500];
//int *chain;
UINT16 tmp; //Create a 16-bit variable to pass to the FIFO
UINT16 tmp2;
int pb_Status=0; //1= =pause, 0 = play
int fileNum = 0;
int next_bp = 0;
int previous_bp = 0;
int pb_mode = 0;
static void button_ISR(void* context, alt_u32 id)
{
	alt_u8 buttons = IORD(BUTTON_PIO_BASE, 3) & 0xF; //get the four least significant bits
	if((buttons & 0x1) != 0 && pb_Status !=1) //if button 1 pressed
	{
		pb_Status = 1; //pause song
		printf("button1pressed\n");
	}
	if((buttons & 0x2) != 0 && pb_Status !=0) //if button 2 pressed
	{
			pb_Status = 0; //play song
			printf("button2pressed\n");
	}
	if((buttons & 0x4) != 0 && pb_Status == 1 && next_bp == 0)// && pb_Status == 1)
	{	printf("button3pressed\n");
		next_bp = 1;
		file_number = file_number%12;
		//file_number=(file_number+1)%12;
		//file_number = file_number - 1;
		search_for_filetype("WAV", &df, 0, 1);

		//file_number--;
		//nextflag=0;
		//LCD.display(df.Name,0);

	}
	if((buttons & 0x8) != 0 && pb_Status == 1 && previous_bp == 0)
	{
		printf("button4pressed");
		previous_bp = 1;

		if(file_number == 1)
			file_number = 11;
		else
			file_number=file_number-2;
		file_number = file_number%12;
		search_for_filetype("WAV", &df, 0, 1);

	}


	IOWR(BUTTON_PIO_BASE, 3, 0x0);
}

void playnormal()
{

	if(pb_Status == 1)
		return;
	next_bp = 0;
	previous_bp = 0;

	LCD_Display(df.Name,(int)pb_mode);

		UINT32 bytesPerCluster = (BPB_BytsPerSec * BPB_SecPerClus); //(bytes/sector)/(sectors/cluster)
		UINT32 length = (1 + ceil(df.FileSize/bytesPerCluster)); //length = 1 + ceil(fileSize/(BPB_SecPerClus * BPB_BytsPerSec));

		//chain = (int *) malloc(length*sizeof(int));
		build_cluster_chain(chain, length, &df);
		//int numSectors = length*BPB_SecPerClus;

		BYTE buffer[512] = {0};

		int i=0;
		int j;

		int numSectors = BPB_SecPerClus * length;

		for(i = 0; i< numSectors; i++)
		{
			if(pb_Status == 1)
				break;

			get_rel_sector(&df, /*(unsigned char *)&*/buffer, chain, i);
			for(j = 0; j < BPB_BytsPerSec; )
			{
				//if(pb_Status == 1)
				//	return;


				if(!IORD( AUD_FULL_BASE, 0 ) ) //Check if the FIFO is not full
				{
					tmp = ( buffer[ j + 1 ] << 8 ) | ( buffer[ j ] ); //Package 2 8-bit bytes from the
					//printf(tmp);
					IOWR( AUDIO_0_BASE, 0, tmp ); //Write the 16-bit variable tmp to the FIFO where it
					j=j+2;

				}

			}
		}


}


void playdouble()
{

	if(pb_Status == 1)
			return;
		next_bp = 0;
		previous_bp = 0;

		LCD_Display(df.Name,(int)pb_mode);

			UINT32 bytesPerCluster = (BPB_BytsPerSec * BPB_SecPerClus); //(bytes/sector)/(sectors/cluster)
			UINT32 length = (1 + ceil(df.FileSize/bytesPerCluster)); //length = 1 + ceil(fileSize/(BPB_SecPerClus * BPB_BytsPerSec));

			//chain = (int *) malloc(length*sizeof(int));
			build_cluster_chain(chain, length, &df);
			//int numSectors = length*BPB_SecPerClus;

			BYTE buffer[512] = {0};

			int i;
			int j;

			int numSectors = BPB_SecPerClus * length;
			int currSector = 1;

			for(i = 0; i< numSectors; i++)
			{
				if(pb_Status == 1)
					break;

				currSector = get_rel_sector(&df, /*(unsigned char *)&*/buffer, chain, i);
				for(j = 0; j < BPB_BytsPerSec; )
				{
					//if(pb_Status == 1)
						//break;


					if(!IORD( AUD_FULL_BASE, 0 ) ) //Check if the FIFO is not full
					{
						tmp = ( buffer[ j + 1 ] << 8 ) | ( buffer[ j ] ); //Package 2 8-bit bytes from the
						//printf(tmp);
						tmp = ( buffer[ j + 3 ] << 8 ) | ( buffer[ j+2 ] );
						IOWR( AUDIO_0_BASE, 0, tmp ); //Write the 16-bit variable tmp to the FIFO where it

						while( IORD( AUD_FULL_BASE, 0 ) ){}
							IOWR( AUDIO_0_BASE, 0, tmp2 );

						j+=8;

					}

				}

			}






}

void playhalf()
{

	if(pb_Status == 1)
			return;
		next_bp = 0;
		previous_bp = 0;

		LCD_Display(df.Name,(int)pb_mode);

			UINT32 bytesPerCluster = (BPB_BytsPerSec * BPB_SecPerClus); //(bytes/sector)/(sectors/cluster)
			UINT32 length = (1 + ceil(df.FileSize/bytesPerCluster)); //length = 1 + ceil(fileSize/(BPB_SecPerClus * BPB_BytsPerSec));

			//chain = (int *) malloc(length*sizeof(int));
			build_cluster_chain(chain, length, &df);
			//int numSectors = length*BPB_SecPerClus;

			BYTE buffer[512] = {0};

			int i;
			int j;

			int numSectors = BPB_SecPerClus * length;
			int currSector = 1;

			int innerLoopCount = 0;

			for(i = 0; i< numSectors; i++)
			{
				if(pb_Status == 1)
					break;

				currSector = get_rel_sector(&df, /*(unsigned char *)&*/buffer, chain, i);
				for(j = 0; j < BPB_BytsPerSec; )
				{
					//if(pb_Status == 1)
						//break;


					if(!IORD( AUD_FULL_BASE, 0 ) ) //Check if the FIFO is not full
					{
						tmp = ( buffer[ j + 1 ] << 8 ) | ( buffer[ j ] ); //Package 2 8-bit bytes from the
						tmp2 = ( buffer[ j + 3 ] << 8 ) | ( buffer[ j+2 ] ); //Package 2 8-bit bytes from the
						IOWR( AUDIO_0_BASE, 0, tmp ); //Write the 16-bit variable tmp to the FIFO where it

						while( IORD( AUD_FULL_BASE, 0 ) ){}
							IOWR( AUDIO_0_BASE, 0, tmp2 );

						while( IORD( AUD_FULL_BASE, 0 ) ){}
							IOWR( AUDIO_0_BASE, 0, tmp );

						//half speed part
						while( IORD( AUD_FULL_BASE, 0 ) ){}
						IOWR( AUDIO_0_BASE, 0, tmp2 ); //Write the 16-bit variable tmp to the FIFO where it
						j+=4;





					}


				}

			}






}


void playreverse()
{

	if(pb_Status == 1)
		return;
	next_bp = 0;
	previous_bp = 0;

	LCD_Display(df.Name,(int)pb_mode);

		UINT32 bytesPerCluster = (BPB_BytsPerSec * BPB_SecPerClus); //(bytes/sector)/(sectors/cluster)
		UINT32 length = (1 + ceil(df.FileSize/bytesPerCluster)); //length = 1 + ceil(fileSize/(BPB_SecPerClus * BPB_BytsPerSec));

		//chain = (int *) malloc(length*sizeof(int));
		build_cluster_chain(chain, length, &df);
		//int numSectors = length*BPB_SecPerClus;

		BYTE buffer[512] = {0};

		int i;
		int j;

		int numSectors = BPB_SecPerClus * length;
		int currSector = 1;

		for(i = ((int)ceil(df.FileSize / BPB_BytsPerSec) - 1);i>=0;i--)//for(i = 0; i<= numSectors; i++)
		{
			if(pb_Status == 1)
				break;

			currSector = get_rel_sector(&df, /*(unsigned char *)&*/buffer, chain, i);
			for(j = (int)BPB_BytsPerSec - 4; j >= 0; )
			{
				//if(pb_Status == 1)
					//break;


				if(!IORD( AUD_FULL_BASE, 0 ) ) //Check if the FIFO is not full
				{
					tmp = ( buffer[ j + 1 ] << 8 ) | ( buffer[ j ] ); //Package 2 8-bit bytes from the
					//printf(tmp);
					tmp2 = ( buffer[ j + 3 ] << 8 ) | ( buffer[ j+2 ] );
					IOWR( AUDIO_0_BASE, 0, tmp ); //Write the 16-bit variable tmp to the FIFO where it

					while( IORD( AUD_FULL_BASE, 0 ) ){}
						IOWR( AUDIO_0_BASE, 0, tmp2 );

					j-=4;

				}

			}

		}



}

playdelay()
{


	if(pb_Status == 1)
			return;
		next_bp = 0;
		previous_bp = 0;



		LCD_Display(df.Name,(int)pb_mode);

			UINT32 bytesPerCluster = (BPB_BytsPerSec * BPB_SecPerClus); //(bytes/sector)/(sectors/cluster)
			UINT32 length = (1 + ceil(df.FileSize/bytesPerCluster)); //length = 1 + ceil(fileSize/(BPB_SecPerClus * BPB_BytsPerSec));

			//chain = (int *) malloc(length*sizeof(int));
			build_cluster_chain(chain, length, &df);
			//int numSectors = length*BPB_SecPerClus;

			BYTE buffer[512] = {0};

			BYTE buffer2[44100] = {0};

			int i=0;
			int j;

			int buffer2_counter = 0;

			int numSectors = BPB_SecPerClus * length;

			for(i = 0; i< numSectors; i++)
			{
				if(pb_Status == 1)
					break;

				get_rel_sector(&df, /*(unsigned char *)&*/buffer, chain, i);
				for(j = 0; j < BPB_BytsPerSec; )
				{
					//if(pb_Status == 1)
					//	return;


					if(!IORD( AUD_FULL_BASE, 0 ) ) //Check if the FIFO is not full
					{
						tmp = ( buffer[ j + 1 ] << 8 ) | ( buffer[ j ] ); //Package 2 8-bit bytes from the
						tmp2 = ( buffer[ j + 3 ] << 8 ) | ( buffer[ j+2 ] );//RIGHT SIDE //Package 2 8-bit bytes from the



						//printf(tmp);
						IOWR( AUDIO_0_BASE, 0, tmp ); //Write the 16-bit variable tmp to the FIFO where it

						while( IORD( AUD_FULL_BASE, 0 ) ){} //write value in buffer
						IOWR( AUDIO_0_BASE, 0, ((BYTE)buffer2[buffer2_counter+1]<<8 | (BYTE)buffer2[buffer2_counter]) ); //Write the 16-bit variable tmp to the FIFO where it

						buffer2[buffer2_counter] =(BYTE) buffer[ j + 2 ];
						buffer2[buffer2_counter+1] = (BYTE) buffer[j+3];

						j=j+4;
						buffer2_counter = (buffer2_counter+2)%44100;

					}

				}
			}
			if(pb_Status == 1)
						return;

			for(i = 0;i<44100;)//with this method we get a strange sound in the delayed speaker, with the 172 method, you don't get this error
			{
				while( IORD( AUD_FULL_BASE, 0 ) ){}
					IOWR( AUDIO_0_BASE, 0, 0 );
				while( IORD( AUD_FULL_BASE, 0 ) ){}
					IOWR( AUDIO_0_BASE, 0, ((BYTE)buffer2[i+1]<<8 | (BYTE)buffer2[i]) );

				i=i+2;
			}



}




int main()
{
	SD_card_init();
	init_mbr();
	init_bs();

	init_audio_codec();
	//printf("hello");

	//BUTTON  ISR--------------------------------------------------------------
				//#ifdef BUTTON_PIO_BASE
				  /* initialize the button PIO */

				  /* direction is input only */

				  /* set up the interrupt vector */ //BUTTON HANDLER
				  alt_irq_register( BUTTON_PIO_IRQ, (void*)0, button_ISR );

				  /* reset the edge capture register by writing to it (any value will do) */
				  IOWR(BUTTON_PIO_BASE, 3, 0x0);

				  /* enable interrupts for the first two buttons*/
				  IOWR(BUTTON_PIO_BASE, 2, 0xF);

				//#endif


		search_for_filetype("WAV", &df, 0, 1);
		int x = 0;
while(1)
{
	pb_mode = IORD(SWITCH_PIO_BASE, 0) & 0x7;

	if(pb_mode == 0)
	{
	playnormal();
	}
	else if(pb_mode== 1)
	{

		playdouble();
	}
	else if(pb_mode == 2)
	{

		playhalf();
	}
	else if(pb_mode == 4)
			playreverse();

	else if(pb_mode == 3)
		playdelay();


	//return 0;
}





}




