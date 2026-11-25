#ifndef MODBUSRTU_H
#define MODBUSRTU_H

#ifdef __cplusplus
 extern "C" {
#endif
 enum Modbus_reg_offset{
     MB_MASTER_VERSION              =0,
     MB_SPARE_VERSION               ,
     MB_SUPPORT_NUM                 ,
     MB_REPORT_INTERVAL             ,
     MB_INPORT_STATUS   	         ,
     MB_OUTPORT_STATUS              ,

     MB_ANALOG_DIGITAL_0            =100,
     MB_ANALOG_DIGITAL_1            ,
     MB_ANALOG_DIGITAL_2            ,
     MB_ANALOG_DIGITAL_3            ,
     MB_ANALOG_DIGITAL_4            ,
     MB_ANALOG_DIGITAL_5            ,
     MB_ANALOG_DIGITAL_6            ,
     MB_ANALOG_DIGITAL_7            ,
     MB_ANALOG_DIGITAL_8            ,
     MB_ANALOG_DIGITAL_9            ,
     MB_ANALOG_DIGITAL_10           ,
     MB_ANALOG_DIGITAL_11           ,
     MB_ANALOG_DIGITAL_12           ,
     MB_ANALOG_DIGITAL_13           ,
     MB_ANALOG_DIGITAL_14           ,
     MB_ANALOG_DIGITAL_15           ,
     MB_MPUTEMP                     ,
     MB_MPUGX                       ,
     MB_MPUGY                       ,
     MB_MPUGZ                       ,
     MB_MPUAX                       ,
     MB_MPUAY                       ,
     MB_MPUAZ                       ,

     MB_SBCCR0					=1000,
     MB_SBCCR1                    ,
     MB_SBCSR0                    ,
     MB_SBCSR1                    ,
     MB_SBCTIME0                  ,
     MB_SBCCNT0                   ,
     MB_ENPX					=2000,
     MB_SWPX					 ,
     MB_SENSORCH                ,
     MB_SENSORTYPE              ,
     MB_SENSORCR0               ,
     MB_SENSORSR0               ,
     MB_SENSORDATAH             ,
     MB_SENSORDATAL             ,

     MB_MAX_OFFSET
 };

 //控制寄存器为u16寄存器
 enum modbus_reg_bit_num{
     BIT0  = 0 ,
     BIT1  = 1 ,
     BIT2,
     BIT3,
     BIT4,
     BIT5,
     BIT6,
     BIT7,
     BIT8,
     BIT9,
     BIT10,
     BIT11,
     BIT12,
     BIT13,
     BIT14,
     BIT15
 };
unsigned char check_xor(void *data, int length);
unsigned short crc16(unsigned char *buffer, unsigned short buffer_length);
int ModbusRTUSingleRegisterWrite( int tty_num, int slave_id, int addr, int value);
int ModbusRTUMultipleRegistersWrite( int ttynum,int slave_id, int start_addr, int reg_count, int *data);
int ModbusRTUMultipleRegistersRead( int ttynum,int slave_id, int start_addr, int count,int *dest, int dest_size);
int InterfaceBoardUploadDataReceive(int ttynum,unsigned char *data,int *length);

#ifdef __cplusplus
}
#endif


#endif
