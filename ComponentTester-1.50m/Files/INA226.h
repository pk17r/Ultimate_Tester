//    FILE: INA226.h
//  AUTHOR: Rob Tillaart
// VERSION: 0.6.0
//    DATE: 2021-05-18
// PURPOSE: Arduino library for INA226 power sensor
//     URL: https://github.com/RobTillaart/INA226
//
//  ADAPTED TO COMPONENT TESTED PROJECT
//      By: Prashant Kumar
//    DATE: 2024-12-23
//
//  Read the datasheet for the details

//  REGISTERS
#define INA226_CONFIGURATION              0x00
#define INA226_SHUNT_VOLTAGE              0x01
#define INA226_BUS_VOLTAGE                0x02
#define INA226_POWER                      0x03
#define INA226_CURRENT                    0x04
#define INA226_CALIBRATION                0x05
#define INA226_MASK_ENABLE                0x06
#define INA226_ALERT_LIMIT                0x07
#define INA226_MANUFACTURER               0xFE
#define INA226_DIE_ID                     0xFF


//  CONFIGURATION MASKS
#define INA226_CONF_RESET_MASK            0x8000
#define INA226_CONF_AVERAGE_MASK          0x0E00
#define INA226_CONF_BUSVC_MASK            0x01C0
#define INA226_CONF_SHUNTVC_MASK          0x0038
#define INA226_CONF_MODE_MASK             0x0007


//  See issue #26
#define INA226_MINIMAL_SHUNT              0.001

#define INA226_MAX_WAIT_MS                600   //  millis



/* ************************************************************************
 *   EOF
 * ************************************************************************ */
