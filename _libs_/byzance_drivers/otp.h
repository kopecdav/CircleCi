
#ifndef OTP_H
#define OTP_H

#include "stm32f4xx.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_flash.h"

#define OTP_START_ADDR		(0x1FFF7800)
#define OTP_LOCK_ADDR		(0x1FFF7A00)
#define OTP_BLOCKS			16
#define OTP_BYTES_IN_BLOCK	32
#define OTP_SIZE			(OTP_BLOCKS * OTP_BYTES_IN_BLOCK)

typedef enum {
    OTPResultOk = 0,
    OTPResultError = -1,
} OTPResult_t;

#define TM_OTP_BlockLocked(block)   ((*(__IO uint8_t *) (OTP_LOCK_ADDR + block)) == 0x00 ? 1 : 0)

class OTP {

public:

    static OTPResult_t write(uint8_t block, uint8_t byte, uint8_t data) {
        HAL_StatusTypeDef status;
        
        /* Check input parameters */
        if (block >= OTP_BLOCKS || byte >= OTP_BYTES_IN_BLOCK) {
            /* Invalid parameters */
            return OTPResultError;
        }

        /* Unlock FLASH */
        HAL_FLASH_Unlock();
        HAL_FLASH_OB_Unlock();
        
        /* Write byte */
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, OTP_START_ADDR + block * OTP_BYTES_IN_BLOCK + byte, data);

        /* Lock FLASH */
        HAL_FLASH_OB_Lock();
        HAL_FLASH_Lock();
        
        /* Check status */
        if (status == HAL_OK) {
            /* Return OK */
            return OTPResultOk;
        } else {
            /* Return error */
            return OTPResultError;
        }
    }

    static uint8_t read(uint8_t block, uint8_t byte) {
        uint8_t data;
        
        /* Check input parameters */
        if (block >= OTP_BLOCKS || byte >= OTP_BYTES_IN_BLOCK) {
            /* Invalid parameters */
            return 0;
        }
        
        /* Get value */
        data = *(__IO uint8_t *)(OTP_START_ADDR + block * OTP_BYTES_IN_BLOCK + byte);
        
        /* Return data */
        return data;
    }

    static OTPResult_t blockLock(uint8_t block) {
//        //TODO: HAL_StatusTypeDef status;
//        
//        /* Check input parameters */
//        if (block >= OTP_BLOCKS) {
//            /* Invalid parameters */
//            
//            /* Return error */
//            return OTPResultError;
//        }
//        
//        /* Unlock FLASH */
//        FLASH_Unlock();
//
//        /* Clear pending flags (if any) */  
//        FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
//                        FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 
//        
//        /* Wait for last operation */
//        status = FLASH_WaitForLastOperation();
//        
//        /* If it is not success, return error */
//        if (status != FLASH_COMPLETE) {
//            /* Lock FLASH */
//            FLASH_Lock();
//            
//            /* Return error */
//            return OTPResultError;
//        }
//        
//        /* Write byte */
//        status = FLASH_ProgramByte(OTP_LOCK_ADDR + block, 0x00);
//
//        /* Lock FLASH */
//        FLASH_Lock();
//        
//        /* Check status */
//        if (status == FLASH_COMPLETE) {
//            /* Return OK */
//            return OTPResultOk;
//        }
//        
//        /* Return error */
        return OTPResultError;
    }

    static bool blockLocked(uint8_t block) {
        return ((*(__IO uint8_t *) (OTP_LOCK_ADDR + block)) == 0x00 ? 1 : 0);
    }

};

#endif /* OTP_H */
