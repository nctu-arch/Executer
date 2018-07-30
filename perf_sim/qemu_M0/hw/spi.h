#ifndef QEMU_SPI_H
#define QEMU_SPI_H

#include "qdev.h"

/* pass to spi_set_cs to deslect all devices on bus */

#define SPI_BUS_NO_CS 0xFF

/* state of SPI device,
 * SPI_NO_CS -> the CS pin in de-asserted -> device is tristated
 * SPI_IDLE -> CS is asserted and device ready to recv
 * SPI_DATA_PENDING ->CS is asserted and the device has pushed data to master
 */

typedef enum{
    SPI_NO_CS,
    SPI_IDLE,
    SPI_DATA_PENDING
}SpiSlaveState;

//typedef int (*init)(SPISlave *s);

/* change the cs pin state */
//typedef void (*cs)(SPISlave *s, uint8_t select);

/* Master to slave. */
//typedef SpiSlaveState (*send)(SPISlave *s, uint32_t data, int len);

/* Slave to master. */
//typedef SpiSlaveState (*recv)(SPISlave *s, uint32_t *data);

/* poll the spi device state */
//typedef SpiSlaveState (*get_state)(SPISlave *s);
//typedef struct SPISlaveInfo{
//    DeviceInfo qdev;

//    /* Callbacks provided by the device. */
//    spi_slave_initfn init;
//    spi_cs_cb cs;
//    spi_recv_cb recv;
//    spi_send_cb send;
//    spi_get_state_cb get_state;
//}SPISlaveInfo;

typedef struct SPISlave{
    DeviceState qdev;
    //SPISlaveInfo *info;
    uint8_t cs;
}SPISlave;

#define TYPE_SPI_SLAVE "spi-slave"
#define SPI_SLAVE(obj) \
    OBJECT_CHECK(SPISlave, (obj), TYPE_SPI_SLAVE)
#define SPI_SLAVE_CLASS(klass)\
    OBJECT_CLASS_CHECK(SPISlaveClass, (klass), TYPE_SPI_SLAVE)
#define SPI_SLAVE_GET_CLASS(obj)\
    OBJECT_GET_CLASS(SPISlaveClass, (obj), TYPE_SPI_SLAVE)




typedef struct SPISlaveClass{
    DeviceClass parent_class;

    /* Callbacks provided by the device. */
    int (*init)(SPISlave *s);

    /* change the cs pin state */
    void (*cs)(SPISlave *s, uint8_t select);

   /* Master to slave. */
    SpiSlaveState (*send)(SPISlave *s, uint32_t data, int len);

    /* Slave to master. */
    SpiSlaveState (*recv)(SPISlave *s, uint32_t *data);

    /* poll the spi device state */
    SpiSlaveState (*get_state)(SPISlave *s);
}SPISlaveClass;

#define SPI_SLAVE_FROM_QDEV(dev) DO_UPCAST(SPISlave, qdev, dev)
#define FROM_SPI_SLAVE(type, dev) DO_UPCAST(type, spi, dev)

extern const VMStateDescription vmstate_spi_slave;

#define VMSTATE_SPI_SLAVE(_field, _state){            \
    .name = (stringify(_field)),                     \
    .size = sizeof(SPISlave),                        \
    .vmsd = &vmstate_spi_slave,                    \
    .flags = VMS_STRUCT,                       \
    .offset = vmstate_offset_value(_state, _field, SPISlave), \
}

typedef struct spi_bus{
    BusState qbus;
    SPISlave **slaves;
    uint8_t num_slaves;
    uint8_t cur_slave;
}spi_bus;

/* create a new spi bus */
spi_bus *spi_init_bus(DeviceState *parent, int num_slaves, const char *name);
int spi_attach_slave(spi_bus *bus, SPISlave *s, int cs);

/* change the chip select. Return 1 on failure. */
int spi_set_cs(spi_bus *bus, int cs);
int spi_get_cs(spi_bus *bus);
SpiSlaveState spi_get_state(spi_bus *bus);

SpiSlaveState spi_send(spi_bus *bus, uint32_t data, int len);
SpiSlaveState spi_recv(spi_bus *bus, uint32_t *data);

//void spi_register_slave(SPISlaveInfo *type);//new

#endif
