
#include "hal_timer.h"
#include "hal_trace.h"
#include "stdio.h"
#include "string.h"
#include "bes_tp_zinitix.h"

//#define ZTW_DEBUG

#ifdef ZTW_DEBUG
#define ZTW_TRACE(n, s, ...) TRACE(n, "[%u]" s, TICKS_TO_MS(hal_sys_timer_get()), ##__VA_ARGS__)
#else
#define ZTW_TRACE(n, s, ...) TRACE_DUMMY(n, s, ##__VA_ARGS__)
#endif

typedef struct this_i2c_dev_ {
    uint32_t          bus;
    uint32_t          addr; //device address
    dev_write         fn_write;
    dev_read          fn_read;
}this_i2c_t;

static this_i2c_t zinitix_dev = {
    .addr = ZTW_I2C_ADDR,
};

static int zinitix_read_reg(uint16_t reg_addr, uint16_t *data, uint16_t size);
static int zinitix_write_reg(uint16_t regaddr, uint16_t data);
static uint16_t get_chip_code(uint16_t regaddr);

#define IIC_RETRY_NUM 2

void ztw_register_i2c_rw(dev_write fn_write, dev_read fn_read)
{
    zinitix_dev.fn_write = fn_write;
    zinitix_dev.fn_read  = fn_read;
}

int32_t ztw_read_coord(tp_sample_t* touch_sample)
{
    ts_zinitix_point_info touch_info = {0};
    int ret = zinitix_read_reg(ZINITIX_POINT_STATUS_REG, (uint16_t *)&touch_info, sizeof(ts_zinitix_point_info));
    if (ret)
    {
        ZTW_TRACE(0,"Failed to read touch info\n");
        return -1;
    }

    uint8_t sub_status = touch_info.coord[0].sub_status;
    if (zinitix_bit_test(sub_status, SUB_BIT_EXIST))
    {
        if (zinitix_bit_test(sub_status, SUB_BIT_DOWN))
        {
            touch_sample->contact = CONTACT_DOWN;
        }
        else if (zinitix_bit_test(sub_status, SUB_BIT_MOVE))
        {
            touch_sample->contact = CONTACT_MOVE;
        }
        touch_sample->x = touch_info.coord[0].x;
        touch_sample->y = touch_info.coord[0].y;

#ifdef  CONFIG_ROTATION
        touch_sample->x = TPD_RES_MAX_X - touch_sample->x;
        touch_sample->y = TPD_RES_MAX_Y - touch_sample->y;        
#endif 
        ZTW_TRACE(0,"1 get point (%d, %d) status %d\n", touch_sample->x, touch_sample->y, sub_status);
    }
    else if (zinitix_bit_test(sub_status, SUB_BIT_UP))
    {
        touch_sample->contact = CONTACT_UP;
        touch_sample->x = touch_info.coord[0].x;
        touch_sample->y = touch_info.coord[0].y;

#ifdef  CONFIG_ROTATION
        touch_sample->x = TPD_RES_MAX_X - touch_sample->x;
        touch_sample->y = TPD_RES_MAX_Y - touch_sample->y;        
#endif
        ZTW_TRACE(0,"2 get point (%d, %d) status %d\n", touch_sample->x, touch_sample->y, sub_status);
    }
    else
    {
        touch_sample->contact = CONTACT_NONE;
    }

    return 0;
}

uint16_t ztw_readchipcode(void)
{
    zinitix_write_reg(ZINITIX_VENDOR_REG, 0x0001);
    uint16_t chip_code = get_chip_code(CHIP_ID_REG);

    return chip_code;
}

static int zinitix_read_reg(uint16_t reg_addr, uint16_t *data, uint16_t size)
{
    int ret = 0;

    this_i2c_t *pdev = &zinitix_dev;

    int retries = 0;

    ZTW_TRACE(0,"i2c read reg: 0x%04x, length:%d \n", reg_addr, size);

    while (retries < IIC_RETRY_NUM)
    {
        ret = zinitix_dev.fn_read(zinitix_dev.bus, zinitix_dev.addr, (uint8_t*)&reg_addr, sizeof(reg_addr), (uint8_t*)data, size);
        if (ret == 0)
          break;
        retries++;
    }

    if (retries >= IIC_RETRY_NUM) {
      ZTW_TRACE(0, "Failed to I2C read\n");
      return -1;
    }

    ZTW_TRACE(0,"i2c addr: 0x%02x regaddr: 0x%02x data: 0x%02x\n", pdev->addr, reg_addr, *data);
    return ret;
}

static int zinitix_write_reg(uint16_t regaddr, uint16_t data)
{
    this_i2c_t *pdev = &zinitix_dev;

    ZTW_TRACE(0,"i2c addr: 0x%02X regaddr: 0x%02X value: 0x%02X\n", pdev->addr, regaddr, data);

    uint8_t msg_buf[32] = {0};

    if(sizeof(data) + sizeof(regaddr) > sizeof(msg_buf))
    {
        TRACE(0," %s length exceed %d",__func__, sizeof(data) + sizeof(regaddr));
        return -1;
    }
    msg_buf[0] = (regaddr) & 0xff; /* reg addr */
    msg_buf[1] = (regaddr >> 8)&0xff;
    memcpy(msg_buf + sizeof(regaddr), &data, sizeof(data));

    int ret = pdev->fn_write(pdev->bus, pdev->addr,(uint8_t*)msg_buf, sizeof(regaddr) + sizeof(data));

    return ret;
}

static int zinitix_write_cmd(uint16_t cmd)
{
    uint16_t data = 1;
    return zinitix_write_reg(cmd, data);
}

uint16_t get_firmware_version()
{
    uint16_t version;
    zinitix_read_reg(ZINITIX_FIRMWARE_VERSION, &version, sizeof(version));

    ZTW_TRACE(0,"firmware version 0x%04x\n", version);

    return version;
}

static uint16_t get_chip_code(uint16_t regaddr)
{
    uint16_t chip_code = 0;

    zinitix_read_reg(regaddr, &chip_code, sizeof(chip_code));

    ZTW_TRACE(0," %s chip_code %x \n",__func__,chip_code);  
    return chip_code;
}

static int zinitix_power_sequnce(uint32_t dev_addr)
{
    uint16_t chip_code;

    if (zinitix_write_reg(ZINITIX_VENDOR_REG, 0x0001) != 0) {
        ZTW_TRACE(0,"zinitix_power_sequnce fail \n");
        return -1;
    }
    /*Delay 10us*/
    hal_sys_timer_delay_us(10);

    chip_code = get_chip_code(CHIP_ID_REG);

    ZTW_TRACE(0,"Chip_code = 0x%02x\n", chip_code);
    if (chip_code != CHIP_ID_VALUE) {
        ZTW_TRACE(0,"Failed to read chip code 0x%04x\n", chip_code);
        return -1;
    }

    /*Delay 10us*/
    hal_sys_timer_delay_us(10);
    if (zinitix_write_cmd(ZINITIX_INTN_CLEAR_CMD) != 0) {
        ZTW_TRACE(0,"Failed to write cmd %04x\n", ZINITIX_INTN_CLEAR_CMD);
        return -1;
    }

    /*Delay 10us*/
    hal_sys_timer_delay_us(10);

    if (zinitix_write_reg(ZINITIX_NVM_REG, 0x0001) != 0) {
        ZTW_TRACE(0,"Failed to write cmd %04x\n", ZINITIX_NVM_REG);
        return -1;
    }
    hal_sys_timer_delay_us(5*1000);

    if (zinitix_write_reg(ZINITIX_PROGRAM_START_CMD, 0x0001) != 0) {
        ZTW_TRACE(0, "%s failed to write cmd %04x\n", __func__, ZINITIX_PROGRAM_START_CMD);
        return -1;
    }
    /*Delay 150ms*/
    hal_sys_timer_delay_us(50*1000);
    return 0;
}


uint16_t get_minor_firmware_version()
{
    uint16_t version;
    zinitix_read_reg(ZINITIX_MINOR_FW_VERSION, &version, sizeof(version));
    ZTW_TRACE(0,"minor firmware version 0x%04x\n", version);
    return version;
}

uint16_t get_reg_data_version()
{
    uint16_t version;
    zinitix_read_reg(ZINITIX_DATA_VERSION_REG, &version, sizeof(version));

    ZTW_TRACE(0,"reg version 0x%04x\n", version);
    return version;
}

int32_t ztw_init(uint32_t dev_addr)
{
    uint8_t i;
    int ret = 0;
    uint16_t eeprom_info, reg_ver, firmware_ver, minor_ver;

    ZTW_TRACE(0," zinitix_power_sequnce :\n");

    ret = zinitix_power_sequnce(dev_addr);
    if (ret < 0) {
        ZTW_TRACE(0,"Failed to software power up ret=0x%x\n",ret);
        return ret;
    }
    ret = zinitix_read_reg(ZINITIX_EEPROM_INFO, &eeprom_info, sizeof(eeprom_info));
    if (ret < 0) {
        TRACE(0,"Failed to read eeprom!\n");
        return -1;
    }

    ZTW_TRACE(0,"eeprom_info =0x%x\n", eeprom_info);

    for (int i = 0; i < 10; i++) {
        ret = zinitix_write_cmd(ZINITIX_SWRESET_CMD);
        if (ret == 0)
            break;
        /*Delay 10ms*/
        hal_sys_timer_delay_us(10*1000);
    }
    zinitix_write_reg(ZINITIX_INT_ENABLE_FLAG, 0);
    //init stage 1 endd

    //init stage 2
    for ( i = 0; i < 10; i++) {
        ret = zinitix_write_cmd(ZINITIX_SWRESET_CMD);
        if (ret == 0)
        break;
        /*Delay 10ms*/
        hal_sys_timer_delay_us(10*1000);
    }

    uint16_t reg_val = 0;
    zinitix_bit_set(reg_val, BIT_PT_CNT_CHANGE);
    zinitix_bit_set(reg_val, BIT_DOWN);
    zinitix_bit_set(reg_val, BIT_MOVE);
    zinitix_bit_set(reg_val, BIT_UP);
    zinitix_bit_set(reg_val, BIT_PALM);
    zinitix_bit_set(reg_val, BIT_PT_EXIST);

    ret = zinitix_read_reg(ZINITIX_EEPROM_INFO, &eeprom_info, sizeof(eeprom_info));
    if (ret < 0) {
        /*Failed to read eeprom info*/
        TRACE(0,"Failed to read eeprom: %04x\n", ZINITIX_EEPROM_INFO);
        return ret;
    }
    ZTW_TRACE(0, "eeprom_info =0x%x\n", eeprom_info);

    zinitix_write_reg(ZINITIX_SUPPORTED_FINGER_NUM, MAX_SUPPORTED_FINGER_NUM);
    zinitix_write_reg(ZINITIX_X_RESOLUTION, (TPD_RES_MAX_X));
    zinitix_write_reg(ZINITIX_Y_RESOLUTION, (TPD_RES_MAX_Y));

    zinitix_write_reg(ZINITIX_INITIAL_TOUCH_MODE, TOUCH_POINT_MODE);
    zinitix_write_reg(ZINITIX_TOUCH_MODE, TOUCH_POINT_MODE);

    zinitix_write_reg(ZINITIX_INT_ENABLE_FLAG, reg_val);
    for (i = 0; i < 10; i++) {
        zinitix_write_cmd(ZINITIX_CLEAR_INT_STATUS_CMD);
        /*Delay 10us*/
        hal_sys_timer_delay_us(10);
    }
    firmware_ver = get_firmware_version();
    minor_ver = get_minor_firmware_version();
    reg_ver= get_reg_data_version();
    //priv->fw_version = firmware_ver;

    ZTW_TRACE(0,"firmware_version =0x%x:0x%x:0x%x\n", firmware_ver, minor_ver, reg_ver);

    // //init stage 3 
    //   ret = config->attach(config, zinitix_data_interrupt);
    //   config->enable(config, true);
    return 0;
}

int32_t ztw_probe(uint32_t i2c_bus)
{
    zinitix_dev.bus = i2c_bus;
    if(ztw_readchipcode() != ZTW523_CHIP_CODE)
    {
        return -1;
    }

    return 0;
}
