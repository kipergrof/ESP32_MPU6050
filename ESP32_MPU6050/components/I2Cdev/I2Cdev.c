#include "driver/i2c.h"
#include "I2Cdev.h"


bool I2Cdev_writeByte(uint8_t devAdd, uint8_t regAddr, uint8_t data) {
	i2c_cmd_handle_t cmd;

	cmd = i2c_cmd_link_create();
	ESP_ERROR_CHECK(i2c_master_start(cmd));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (devAdd << 1) | I2C_MASTER_WRITE, 1));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, regAddr, 1));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, data, 1));
	ESP_ERROR_CHECK(i2c_master_stop(cmd));
	ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS));
	i2c_cmd_link_delete(cmd);

	return true;
}

void I2Cdev_SelectRegister(uint8_t dev, uint8_t reg){
	i2c_cmd_handle_t cmd;

	cmd = i2c_cmd_link_create();
	ESP_ERROR_CHECK(i2c_master_start(cmd));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (dev << 1) | I2C_MASTER_WRITE, 1));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, reg, 1));
	ESP_ERROR_CHECK(i2c_master_stop(cmd));
	ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS));
	i2c_cmd_link_delete(cmd);
}







bool I2Cdev_writeBytes(uint8_t devAdd, uint8_t regAddr, uint8_t length, uint8_t *data){
	i2c_cmd_handle_t cmd;

	cmd = i2c_cmd_link_create();
	ESP_ERROR_CHECK(i2c_master_start(cmd));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (devAdd << 1) | I2C_MASTER_WRITE, 1));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, regAddr, 1));
	ESP_ERROR_CHECK(i2c_master_write(cmd, data, length-1, 0));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, data[length-1], 1));
	ESP_ERROR_CHECK(i2c_master_stop(cmd));
	ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS));
	i2c_cmd_link_delete(cmd);
	return true;
}

uint8_t I2Cdev_readBytes(uint8_t devAdd, uint8_t regAddr, uint8_t length, uint8_t *data) {
	i2c_cmd_handle_t cmd;
	I2Cdev_SelectRegister(devAdd, regAddr); //

	cmd = i2c_cmd_link_create();
	ESP_ERROR_CHECK(i2c_master_start(cmd));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (devAdd << 1) | I2C_MASTER_READ, 1));

	if(length>1)
		ESP_ERROR_CHECK(i2c_master_read(cmd, data, length-1, 0));

	ESP_ERROR_CHECK(i2c_master_read_byte(cmd, data+length-1, 1));

	ESP_ERROR_CHECK(i2c_master_stop(cmd));
	ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS));
	i2c_cmd_link_delete(cmd);

	return length;
}

uint8_t I2Cdev_readByte(uint8_t devAdd, uint8_t regAddr, uint8_t *data) {
    return I2Cdev_readBytes(devAdd, regAddr, 1, data);
}

uint8_t I2Cdev_readBits(uint8_t devAdd, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data) {

    uint8_t count, b;
    if ((count = I2Cdev_readByte(devAdd, regAddr, &b)) != 0) {
        uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        b &= mask;
        b >>= (bitStart - length + 1);
        *data = b;
    }
    return count;
}

bool I2Cdev_writeBits(uint8_t devAdd, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data) {
	uint8_t b = 0;
    if (I2Cdev_readByte(devAdd, regAddr, &b) != 0) {
        uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        data <<= (bitStart - length + 1); // shift data into correct position
        data &= mask; // zero all non-important bits in data
        b &= ~(mask); // zero all important bits in existing byte
        b |= data; // combine data with existing byte
        return I2Cdev_writeByte(devAdd, regAddr, b); //
    } else {
        return false;
    }
}

bool I2Cdev_writeBit(uint8_t devAdd, uint8_t regAddr, uint8_t bitNum, uint8_t data) {
    uint8_t b;
    I2Cdev_readByte(devAdd, regAddr, &b);
    b = (data != 0) ? (b | (1 << bitNum)) : (b & ~(1 << bitNum));
    return I2Cdev_writeByte(devAdd, regAddr, b);
}





uint8_t I2Cdev_readWord(uint8_t devAdd, uint8_t regAddr, uint16_t *data){
	uint8_t msb[2] = {0,0};
	I2Cdev_readBytes(devAdd, regAddr, 2, msb);
	*data = (int16_t)((msb[0] << 8) | msb[1]);
	return 0;
}

bool I2Cdev_writeWord(uint8_t devAddr, uint8_t regAddr, uint16_t data){

	uint8_t data1[] = {(uint8_t)(data>>8), (uint8_t)(data & 0xff)};
	uint8_t data2[] = {(uint8_t)(data & 0xff), (uint8_t)(data>>8)};
	I2Cdev_writeBytes(devAddr, regAddr, 2, data1);
	return true;
}


void I2Cdev_init()
{
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = (gpio_num_t)PIN_SDA;
	conf.scl_io_num = (gpio_num_t)PIN_CLK;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = 400000;
	ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
	ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));
}






