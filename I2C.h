void I2C_Init();
void I2C_Wait(void);
void I2C_Start(void);
void I2C_ReStart(void);
void I2C_Stop(void);
char I2C_Read(char);
void I2C_Write(unsigned char);
void I2C_Write_Cmd_Only(char, char);  
void I2C_Write_Cmd_Write_Data(char, char, char);
void I2C_Write_Address_Write_One_Byte(char, char, char);    
BYTE I2C_Write_Address_Read_One_Byte(char, char);
 
void i2cHighSda();
void i2cLowSda();
void i2cHighScl();
void i2cLowScl();
void i2cNack();
void i2cAck();

