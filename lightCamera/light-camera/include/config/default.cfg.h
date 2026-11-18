#pragma once

#ifndef PRODUCTION
    #define PRODUCTION 1
#endif /* PRODUCTION */

#ifndef DEBUG
    #define DEBUG 1
#endif /* DEBUG */

#ifndef SENSOR_HILIM
    #define SENSOR_HILIM  20
#endif /* SENSOR_HILIM */

#ifndef SENSOR_LOLIM
    #define SENSOR_LOLIM  0
#endif /* SENSOR_LOLIM */

#ifndef PIN_INT
    #define PIN_INT GPIO_NUM_13
#endif /* PIN_INT */

#ifndef DEBUG_LED_PIN
    #define DEBUG_LED_PIN 33
#endif /* DEBUG_LED_PIN */

#ifndef PIN_SDA
    #define PIN_SDA 14
#endif /* PIN_SDA */

#ifndef PIN_SCL
    #define PIN_SCL 15
#endif /* PIN_SCL */
