/*
 *******************************************************************
 *						Audio Common Definitions
 *
 *Note(s):
 *		 (1)Only macro definitions can be defined here.
 *		 (2)Use (1UL << (n)) instead of BIT(n)
 *******************************************************************
 */
#ifndef _AUDIO_DEF_H_
#define _AUDIO_DEF_H_

/*
 *******************************************************************
 *						DAC Definitions
 *******************************************************************
 */

/*
 *******************************************************************
 *						ADC Definitions
 *******************************************************************
 */


/*
 *******************************************************************
 *						Linein(Aux) Definitions
 *******************************************************************
 */


/*
 *******************************************************************
 *						ANC Definitions
 *******************************************************************
 */
//ANC Mode Enable
#define ANC_FF_EN 		 			(1UL << (0))
#define ANC_FB_EN  					(1UL << (1))
#define ANC_HYBRID_EN  			 	(1UL << (2))

//ANC芯片版本定义
#define ANC_VERSION_BR30 			0x01	//AD697N/AC897N
#define ANC_VERSION_BR30C			0x02	//AC699N/AD698N
#define ANC_VERSION_BR36			0x03	//AC700N
#define ANC_VERSION_BR28			0x04	//AC701N/BR40

#endif/*_AUDIO_DEF_H_*/
