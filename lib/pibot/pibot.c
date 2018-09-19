
#include "../lualib.h"
#include "../../lua-api.h"
#include "utils.h"


#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 500000;
static uint16_t delay = 110;

static int fd_spi = -1;

int spi_open()
{
    int ret;
    int fd = open(device, O_RDWR);
	if (fd < 0)
	{
		perror("can't open device \n");
		return -1;
	}
	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
	{
	    perror("can't set spi mode \n");
	    return -1;
	}

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
	{
		perror("can't get spi mode \n");
		return -1;
	}
	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
	{
		perror("can't set bits per word \n");
		return -1;
	}

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
	{
	    perror("can't get bits per word");
	    return -1;
	}
	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
	{
	    perror("can't set max speed hz");
	    return -1;
	}
	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
	{
		perror("can't get max speed hz");
		return -1;
	}

	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
	
	return fd;
}

int spi_send_cmd(int fd, uint8_t cmd, uint8_t idx, uint8_t value)
{
    int ret;
    uint8_t tx[3]; 
    uint8_t rx[3] = {0, };
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = 3,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits
	};
	tx[0] = cmd;
	tx[1] = idx;
	tx[2] = value;
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1)
    {
        perror("can't send spi message");
    	return -1;
    }
    //if(cmd == 255)
    // printf("RX %d %d %d \n", rx[0], rx[1], rx[2]);
    return (int) rx[0];
}

int spi_set(int fd,uint8_t idx, uint8_t v)
{
    return spi_send_cmd(fd,1, idx, v);
}

int spi_get(int fd,uint8_t idx)
{
    // send command
    int ret;
    ret = spi_send_cmd(fd,0,idx,0);
    if(ret == -1) return -1;
    // read back
    return spi_send_cmd(fd,255,255,255);
}

void spi_read_buff(int fd,uint8_t* buf, int size)
{
    int ret;
    uint8_t tx[size];
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)buf,
		.len = size,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits
	};
	spi_send_cmd(fd,2,0,size);
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1)
    {
        perror("can't send spi message");
    	return;
    }
}

void spi_write_buff(int fd, uint8_t* buf, int size)
{
    int i;
    for(i=0; i < size; i++)
        spi_set(fd,i,buf[i]);
    
}

static int l_init(lua_State* L)
{
    fd_spi = spi_open();
    if(fd_spi == -1)
        lua_pushboolean(L,0);
    else
        lua_pushboolean(L,1);
    return 1;
}

static int l_release(lua_State* L)
{
    if(fd_spi > 0)
        close(fd_spi);
    fd_spi = -1;
    lua_pushboolean(L,1);
    return 1;
}

static int l_read(lua_State* L)
{
    int size = luaL_checknumber(L,1);
    unsigned char data_buf[size];
    if(fd_spi == -1)
    {
        lua_pushnil(L);
        return 1;
    }
    // request data
	spi_read_buff(fd_spi,data_buf,size);
    
    lua_newtable(L);
    
    int i;
    for(i =0; i < size; i++)
    {
        lua_pushnumber(L,i);
	    lua_pushnumber(L,data_buf[i]);
	    lua_settable(L,-3);
    }
    return 1;
}

static int l_write(lua_State* L)
{
    /* 1st argument must be a table (t) */
    byte_array_t* data_buf = l_check_barray(L, 1);
    if(fd_spi == -1)
    {
        lua_pushboolean(L,0);
        return 1;
    }
    spi_write_buff(fd_spi,data_buf->data, data_buf->size);
    lua_pushboolean(L,1);
    return 1;
}

static int l_set(lua_State* L)
{
    int idx = luaL_checknumber(L,1);
    int val = luaL_checknumber(L,2);
    if(fd_spi == -1)
    {
        lua_pushboolean(L,0);
        return 1;
    }
    spi_set(fd_spi,idx,val);
    lua_pushboolean(L,1);
    return 1;
}

static int l_get(lua_State* L)
{
    int idx = luaL_checknumber(L,1);
    if(fd_spi == -1)
    {
        lua_pushboolean(L,0);
        return 1;
    }
    spi_get(fd_spi,idx);
    lua_pushboolean(L,1);
    return 1;
}

static const struct luaL_Reg _lib [] = {
	{"init", l_init},
	{"release",l_release},
	{"read",l_read},
	{"write",l_write},
	{"set",l_set},
	{"get",l_get},
	{NULL,NULL}
};

int luaopen_pibot(lua_State *L)
{
	luaL_newlib(L, _lib);
	return 1;
}
