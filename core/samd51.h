/**
 * \file
 *
 * \brief Top header file for SAMD51
 *
 * Copyright (c) 2019 Microchip Technology Inc.
 *
 * \asf_license_start
 *
 * \page License
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the Licence at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * \asf_license_stop
 *
 */

#ifndef _SAMD51_
#define _SAMD51_

/**
 * \defgroup SAMD51_definitions SAMD51 Device Definitions
 * \brief SAMD51 CMSIS Definitions.
 */

#if   defined(__SAMD51G18A__) || defined(__ATSAMD51G18A__)
  #include "samd51g18a.h"
#elif defined(__SAMD51G19A__) || defined(__ATSAMD51G19A__)
  #include "samd51g19a.h"
#elif defined(__SAMD51J18A__) || defined(__ATSAMD51J18A__)
  #include "samd51j18a.h"
#elif defined(__SAMD51J19A__) || defined(__ATSAMD51J19A__)
  #include "samd51j19a.h"
#elif defined(__SAMD51J19B__) || defined(__ATSAMD51J19B__)
  #include "samd51j19b.h"
#elif defined(__SAMD51J20A__) || defined(__ATSAMD51J20A__)
  #include "samd51j20a.h"
#elif defined(__SAMD51J20C__) || defined(__ATSAMD51J20C__)
  #include "samd51j20c.h"
#elif defined(__SAMD51N19A__) || defined(__ATSAMD51N19A__)
  #include "samd51n19a.h"
#elif defined(__SAMD51N20A__) || defined(__ATSAMD51N20A__)
  #include "samd51n20a.h"
#elif defined(__SAMD51P19A__) || defined(__ATSAMD51P19A__)
  #include "samd51p19a.h"
#elif defined(__SAMD51P20A__) || defined(__ATSAMD51P20A__)
  #include "samd51p20a.h"
#else
  #error Library does not support the specified device.
#endif

#include "util.h"

#define PA00 GPIO(GPIO_PORTA, 0)
#define PA01 GPIO(GPIO_PORTA, 1)
#define PA02 GPIO(GPIO_PORTA, 2)
#define PA03 GPIO(GPIO_PORTA, 3)
#define PA04 GPIO(GPIO_PORTA, 4)
#define PA05 GPIO(GPIO_PORTA, 5)
#define PA06 GPIO(GPIO_PORTA, 6)
#define PA07 GPIO(GPIO_PORTA, 7)
#define PA08 GPIO(GPIO_PORTA, 8)
#define PA09 GPIO(GPIO_PORTA, 9)
#define PA10 GPIO(GPIO_PORTA, 10)
#define PA11 GPIO(GPIO_PORTA, 11)
#define PA12 GPIO(GPIO_PORTA, 12)
#define PA13 GPIO(GPIO_PORTA, 13)
#define PA14 GPIO(GPIO_PORTA, 14)
#define PA15 GPIO(GPIO_PORTA, 15)
#define PA16 GPIO(GPIO_PORTA, 16)
#define PA17 GPIO(GPIO_PORTA, 17)
#define PA18 GPIO(GPIO_PORTA, 18)
#define PA19 GPIO(GPIO_PORTA, 19)
#define PA20 GPIO(GPIO_PORTA, 20)
#define PA21 GPIO(GPIO_PORTA, 21)
#define PA22 GPIO(GPIO_PORTA, 22)
#define PA23 GPIO(GPIO_PORTA, 23)
#define PA24 GPIO(GPIO_PORTA, 24)
#define PA25 GPIO(GPIO_PORTA, 25)
#define PA26 GPIO(GPIO_PORTA, 26)
#define PA27 GPIO(GPIO_PORTA, 27)
#define PA28 GPIO(GPIO_PORTA, 28)
#define PA29 GPIO(GPIO_PORTA, 29)
#define PA30 GPIO(GPIO_PORTA, 30)
#define PA31 GPIO(GPIO_PORTA, 31)

#define PB00 GPIO(GPIO_PORTB, 0)
#define PB01 GPIO(GPIO_PORTB, 1)
#define PB02 GPIO(GPIO_PORTB, 2)
#define PB03 GPIO(GPIO_PORTB, 3)
#define PB04 GPIO(GPIO_PORTB, 4)
#define PB05 GPIO(GPIO_PORTB, 5)
#define PB06 GPIO(GPIO_PORTB, 6)
#define PB07 GPIO(GPIO_PORTB, 7)
#define PB08 GPIO(GPIO_PORTB, 8)
#define PB09 GPIO(GPIO_PORTB, 9)
#define PB10 GPIO(GPIO_PORTB, 10)
#define PB11 GPIO(GPIO_PORTB, 11)
#define PB12 GPIO(GPIO_PORTB, 12)
#define PB13 GPIO(GPIO_PORTB, 13)
#define PB14 GPIO(GPIO_PORTB, 14)
#define PB15 GPIO(GPIO_PORTB, 15)
#define PB16 GPIO(GPIO_PORTB, 16)
#define PB17 GPIO(GPIO_PORTB, 17)
#define PB18 GPIO(GPIO_PORTB, 18)
#define PB19 GPIO(GPIO_PORTB, 19)
#define PB20 GPIO(GPIO_PORTB, 20)
#define PB21 GPIO(GPIO_PORTB, 21)
#define PB22 GPIO(GPIO_PORTB, 22)
#define PB23 GPIO(GPIO_PORTB, 23)
#define PB24 GPIO(GPIO_PORTB, 24)
#define PB25 GPIO(GPIO_PORTB, 25)
#define PB26 GPIO(GPIO_PORTB, 26)
#define PB27 GPIO(GPIO_PORTB, 27)
#define PB28 GPIO(GPIO_PORTB, 28)
#define PB29 GPIO(GPIO_PORTB, 29)
#define PB30 GPIO(GPIO_PORTB, 30)
#define PB31 GPIO(GPIO_PORTB, 31)

#define PC00 GPIO(GPIO_PORTC, 0)
#define PC01 GPIO(GPIO_PORTC, 1)
#define PC02 GPIO(GPIO_PORTC, 2)
#define PC03 GPIO(GPIO_PORTC, 3)
#define PC04 GPIO(GPIO_PORTC, 4)
#define PC05 GPIO(GPIO_PORTC, 5)
#define PC06 GPIO(GPIO_PORTC, 6)
#define PC07 GPIO(GPIO_PORTC, 7)
#define PC08 GPIO(GPIO_PORTC, 8)
#define PC09 GPIO(GPIO_PORTC, 9)
#define PC10 GPIO(GPIO_PORTC, 10)
#define PC11 GPIO(GPIO_PORTC, 11)
#define PC12 GPIO(GPIO_PORTC, 12)
#define PC13 GPIO(GPIO_PORTC, 13)
#define PC14 GPIO(GPIO_PORTC, 14)
#define PC15 GPIO(GPIO_PORTC, 15)
#define PC16 GPIO(GPIO_PORTC, 16)
#define PC17 GPIO(GPIO_PORTC, 17)
#define PC18 GPIO(GPIO_PORTC, 18)
#define PC19 GPIO(GPIO_PORTC, 19)
#define PC20 GPIO(GPIO_PORTC, 20)
#define PC21 GPIO(GPIO_PORTC, 21)
#define PC22 GPIO(GPIO_PORTC, 22)
#define PC23 GPIO(GPIO_PORTC, 23)
#define PC24 GPIO(GPIO_PORTC, 24)
#define PC25 GPIO(GPIO_PORTC, 25)
#define PC26 GPIO(GPIO_PORTC, 26)
#define PC27 GPIO(GPIO_PORTC, 27)
#define PC28 GPIO(GPIO_PORTC, 28)
#define PC29 GPIO(GPIO_PORTC, 29)
#define PC30 GPIO(GPIO_PORTC, 30)
#define PC31 GPIO(GPIO_PORTC, 31)

#define PD00 GPIO(GPIO_PORTD, 0)
#define PD01 GPIO(GPIO_PORTD, 1)
#define PD02 GPIO(GPIO_PORTD, 2)
#define PD03 GPIO(GPIO_PORTD, 3)
#define PD04 GPIO(GPIO_PORTD, 4)
#define PD05 GPIO(GPIO_PORTD, 5)
#define PD06 GPIO(GPIO_PORTD, 6)
#define PD07 GPIO(GPIO_PORTD, 7)
#define PD08 GPIO(GPIO_PORTD, 8)
#define PD09 GPIO(GPIO_PORTD, 9)
#define PD10 GPIO(GPIO_PORTD, 10)
#define PD11 GPIO(GPIO_PORTD, 11)
#define PD12 GPIO(GPIO_PORTD, 12)
#define PD13 GPIO(GPIO_PORTD, 13)
#define PD14 GPIO(GPIO_PORTD, 14)
#define PD15 GPIO(GPIO_PORTD, 15)
#define PD16 GPIO(GPIO_PORTD, 16)
#define PD17 GPIO(GPIO_PORTD, 17)
#define PD18 GPIO(GPIO_PORTD, 18)
#define PD19 GPIO(GPIO_PORTD, 19)
#define PD20 GPIO(GPIO_PORTD, 20)
#define PD21 GPIO(GPIO_PORTD, 21)
#define PD22 GPIO(GPIO_PORTD, 22)
#define PD23 GPIO(GPIO_PORTD, 23)
#define PD24 GPIO(GPIO_PORTD, 24)
#define PD25 GPIO(GPIO_PORTD, 25)
#define PD26 GPIO(GPIO_PORTD, 26)
#define PD27 GPIO(GPIO_PORTD, 27)
#define PD28 GPIO(GPIO_PORTD, 28)
#define PD29 GPIO(GPIO_PORTD, 29)
#define PD30 GPIO(GPIO_PORTD, 30)
#define PD31 GPIO(GPIO_PORTD, 31)


extern uint32_t function_pins[3];

#endif /* _SAMD51_ */

