#include <stdio.h>
#include <bits/types.h>
#include <sys/types.h>
#include "./libc/libintf.h"

#ifdef __cplusplus
 extern "C" {
#endif

 typedef __uint8_t uint8_t;
 typedef __uint16_t uint16_t;
 typedef __uint32_t uint32_t;
 typedef __uint64_t uint64_t;


 /* Table of CRC values for high-order byte */
 static const uint8_t table_crc_hi[] = {
     0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
     0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
     0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
     0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
     0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
     0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
     0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
     0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
     0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
     0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
     0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
     0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
     0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
     0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
     0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
     0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
     0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
     0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
     0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
     0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
     0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
     0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
     0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
     0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
     0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
     0x80, 0x41, 0x00, 0xC1, 0x81, 0x40};

 /* Table of CRC values for low-order byte */
 static const uint8_t table_crc_lo[] = {
     0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
     0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
     0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
     0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
     0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
     0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
     0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
     0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
     0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
     0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
     0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
     0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
     0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
     0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
     0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
     0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
     0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
     0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
     0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
     0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
     0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
     0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
     0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
     0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
     0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
     0x43, 0x83, 0x41, 0x81, 0x80, 0x40};

 uint16_t crc16(uint8_t *buffer, uint16_t buffer_length)
 {
     uint8_t crc_hi = 0xFF; /* high CRC byte initialized */
     uint8_t crc_lo = 0xFF; /* low CRC byte initialized */
     unsigned int i;        /* will index into CRC lookup */

     /* pass through message buffer */
     while (buffer_length--)
     {
         i = crc_hi ^ *buffer++; /* calculate the CRC  */
         crc_hi = crc_lo ^ table_crc_hi[i];
         crc_lo = table_crc_lo[i];
     }

     return (crc_hi << 8 | crc_lo);
 }

 unsigned char check_xor(void *data, int length)
 {
     unsigned char xorsum = 0;
     unsigned char *p = (unsigned char *)data;
     while(length--){
         xorsum ^= *p++;
     }
     return xorsum;
 }

 void modbus_query( unsigned char *packet , size_t string_length )
 {
     uint16_t temp_crc;

     temp_crc = crc16( packet, string_length );

     packet[ string_length++ ] = temp_crc >> 8;
     packet[ string_length++ ] = temp_crc & 0x00FF;
     packet[ string_length ] = 0;

 }



 int send_query( int tty_num, unsigned char *query, size_t string_length )
 {

     modbus_query( query, string_length );
     string_length += 2;
 #if 0
     int debugn = 0;



     for(debugn = 0;debugn < string_length;debugn++)
     {
         printf("%x ",query[debugn]);
     }
     printf("\n");
 #endif

     return set_uart_data( tty_num, query, string_length );
 }
#if 1



 int receive_response(  unsigned char *received_string,int dest_size, int ttynum )
 {

     char rxchar = 0;
     int bytes_received = 0;
     int data_avail = 1;
     //int i = 0;
     int read_stat;
     int timeout = 1;

     fd_set rfds;

     struct timeval tv;

     tv.tv_sec = timeout;
     tv.tv_usec = 0;

     FD_ZERO( &rfds );
     FD_SET( g_uart_fd[ttynum], &rfds );

     /* wait for a response */
     data_avail = select( FD_SETSIZE, &rfds, NULL, NULL, &tv );

     if( !data_avail )
     {
         bytes_received = 0;
         fprintf( stderr, "Comms time out\n" );
     }


     tv.tv_sec = 0;
     tv.tv_usec = B115200_SELECT_US;//根据波特率调整

     FD_ZERO( &rfds );
     FD_SET( g_uart_fd[ttynum], &rfds );

     while(data_avail)
     {
         if( select( FD_SETSIZE, &rfds, NULL, NULL, &tv ) )
         {
             read_stat = get_uart_data( ttynum, &rxchar, 1 );

             if( read_stat < 0 )
             {
                 bytes_received = 0;
                 data_avail = 0;
             }
             else
             {
                 rxchar = rxchar & 0xFF;
                 received_string[ bytes_received ++ ] = rxchar;
             }

             if(bytes_received >= dest_size)
             {
                 data_avail = 0;
             }
         }
         else
         {
             data_avail = 0;
         }
     }

 #if 0
 printf("bytes rev %d\n",bytes_received);
     for(i = 0;i < bytes_received;i++)
     {
         printf("%x ",received_string[i]);
     }
     printf("\n");
 #endif

     return( bytes_received );
 }

 int modbus_response( unsigned char *data, int dest_size,unsigned char *query, int ttynum )
 {
     int response_length;

     unsigned short crc_calc = 0;
     unsigned short crc_received = 0;
     unsigned char recv_crc_hi;
     unsigned char recv_crc_lo;


     response_length = receive_response( data,dest_size, ttynum );
     if( response_length )
     {
         crc_calc = crc16( data, response_length - 2 );

         recv_crc_hi = (unsigned) data[ response_length - 2 ];
         recv_crc_lo = (unsigned) data[ response_length - 1 ];

         crc_received = data[ response_length - 2 ];
         crc_received = (unsigned) crc_received << 8;
         crc_received =
               crc_received |(unsigned)data[response_length -1];


         /*********** check CRC of response ************/

         if (crc_calc != crc_received)
         {
             printf( "crc error received \n");
             response_length = 0;
         }



         /********** check for exception response *****/

         if( response_length && data[ 1 ] != query [ 1 ] )
         {
             printf( "function err \n");
             response_length = 0;
         }
     }
     return( response_length );
 }



 int read_reg_response( int *dest, int dest_size, unsigned char *query, int ttynum )
 {
     unsigned char data[ 32 ];
     int raw_response_length = 0;
     int temp,i;



     raw_response_length = modbus_response( data, dest_size,query, ttynum );
     if( raw_response_length > 0 )//减掉crc
         raw_response_length -= 2;


     if( raw_response_length > 0 )
     {
         for( i = 0; i < (data[2] / 2) && i < (raw_response_length); i++ )
         {
             /* shift reg hi_byte to temp */
             temp = data[ 3 + i *2 ] << 8;
             /* OR with lo_byte           */
             temp = temp | data[ 3 + i * 2 + 1 ];

             dest[i] = temp;
         }
     }

     if( raw_response_length > 0 )//减掉地址域和功能码
     {
         raw_response_length -= 3;
         raw_response_length /= 2;
     }

    return( raw_response_length );
}
void build_request_packet( int slave_id, int function, int start_addr,
               int count, unsigned char *packet )
{
    packet[ 0 ] = slave_id,
    packet[ 1 ] = function;
    packet[ 2 ] = start_addr >> 8;
    packet[ 3 ] = start_addr & 0x00ff;
    packet[ 4 ] = count >> 8;
    packet[ 5 ] = count &0x00ff;

}

int read_IO_status(int ttynum,  int function, int slave_id, int start_addr, int count,
                int *dest, int dest_size)
{
    unsigned char packet[256];
    build_request_packet( slave_id, function, start_addr, count, packet );

    if( send_query( ttynum, packet, 256 ) > -1 )
    {
        return read_reg_response( dest, dest_size, packet, ttynum );
    }
    else
    {
        return -1;
    }

}


 int preset_response( unsigned char *query, int ttynum )
 {
     unsigned char data[ 32 ];
     int raw_response_length;

     raw_response_length = modbus_response( data,32, query, ttynum );

     return( raw_response_length );
 }
#endif

 int ModbusRTUSingleRegisterWrite( int tty_num, int slave_id, int addr, int value)
 {
     //unsigned char packet[ REQUEST_QUERY_SIZE ];
     int ret = 0;
     unsigned char packet[ 32 ];
     packet[ 0 ] = slave_id;
     packet[ 1 ] = 06;
     packet[ 2 ] = (addr >> 8) & 0xff;
     packet[ 3 ] = addr & 0x00FF;
     packet[ 4 ] = (value >> 8) & 0xff;
     packet[ 5 ] = value & 0x00FF;

     if( send_query( tty_num, packet, 6 ) > -1 )
     {
         //ret = preset_response( packet, tty_num );
     }
     else
     {
         ret = -1;
     }

     return ret;
 }



 int ModbusRTUMultipleRegistersWrite( int ttynum,int slave_id, int start_addr,
                 int reg_count, int *data)
 {
     int byte_count, i, packet_size = 6;
     int ret = 0;

     unsigned char packet[32];


     packet[ 0 ] = slave_id;
     packet[ 1 ] = 0x10;
     packet[ 2 ] = start_addr >> 8;
     packet[ 3 ] = start_addr & 0x00FF;
     packet[ 4 ] = reg_count >> 8;
     packet[ 5 ] = reg_count & 0x00FF;
     byte_count = reg_count * 2;
     packet[ 6 ] = byte_count;

     for( i = 0; i < reg_count; i++ )
     {
         packet[ ++packet_size ] = data[ i ] >> 8;
         packet[ ++packet_size ] = data[ i ] & 0x00FF;
     }

     //for(debugn = 0;debugn < packet_size;debugn++)
     //{
        // printf("%x ",packet[debugn]);
     //}
     //printf("\n");
     if( send_query( ttynum, packet, ++packet_size ) > -1 )
     {
         //ret = preset_response( packet, ttynum );
     }
     else
     {
         ret = -1;
     }
     return ret;

 }



 int ModbusRTUReadCoilStatus( int ttynum,int slave_id, int start_addr, int count,
               int *dest, int dest_size )
 {
     int function = 0x01;
     int status;

     status = read_IO_status( ttynum,function, slave_id, start_addr, count,
                         dest, dest_size);

     return( status );
 }




 int ModbusRTUMultipleRegistersRead( int ttynum,int slave_id, int start_addr, int count,
               int *dest, int dest_size)
 {

     unsigned char packet[ 64];
     build_request_packet( slave_id, 0x03, start_addr, count, packet );

     if( send_query(ttynum, packet, 6 ) > -1 )
     {
         return read_reg_response( dest, dest_size, packet, ttynum );
     }
     else
     {
         return -1;
     }


 }



 //主动上报接收函数
 int InterfaceBoardUploadDataReceive(int ttynum,unsigned char *data,int *length)
 {
     int read_stat;
     int data_len,i;
     int leng;
     unsigned char header[3] = {0};
     read_stat = get_uart_data(ttynum, header, 3 );
     if(read_stat <= 0)
     {
         //printf( "1header err \n");

         return -1;
     }
     if(header[0] == 0xeb && header[1] == 0x90)
     {
         data_len = header[2];
     }
     else
     {
         //printf( "header err \n");

         return 0;//非主动上报报文
     }

     for(i = 0;i < 3;i++)
     {
         data[i] = header[i];
     }


     read_stat = get_uart_data(ttynum,&data[3],data_len-3);
     if(read_stat <= 0)
     {
         //printf( "3header err \n");

         return -1;
     }

     if(read_stat < data_len-3)
     {
         leng = read_stat+3;
     }
     else
     {
         leng = data_len;
     }
    *length = leng;
#if 0
     for(i = 0;i < leng;i++)
     {
         printf("%x ",data[i]);
     }
     printf("\n");
     //printf("xor %x\n",check_xor(data, (leng-1)));
#endif
     if(check_xor(data, (leng-1)) != data[leng-1])
     {
 #if 0
         printf( "4header err \n");
         for(i = 0;i < leng;i++)
         {
             printf("%x ",data[i]);
         }
         printf("\n");
         printf("xor %x\n",check_xor(data, (leng-1)));
#endif
         return -1;
     }

     return 0;
 }


#ifdef __cplusplus
}
#endif

