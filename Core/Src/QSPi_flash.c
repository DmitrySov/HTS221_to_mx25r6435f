	/*
	 * QSPI_flash.c
	 *
	 *  Created on: 21 мар. 2022 г.
	 *      Author: sovetnikov
	 */
#include "main.h"
#include "QSPI_flash.h"
#include "mx25r6435f.h"

static uint8_t QSPI_WriteEnable(void);
static uint8_t QSPI_AutoPollingMemReady(uint32_t timeout);
static uint8_t QSPI_Configuration(void);
static uint8_t QSPI_ResetChip(void);
/* USER CODE END 0 */

 QSPI_HandleTypeDef hqspi;

	/**
	  * @brief QUADSPI Initialization Function
	  * @param None
	  * @retval None
	  */
	 void MX_QUADSPI_Init(void)
	{

	  /* USER CODE BEGIN QUADSPI_Init 0 */

	  /* USER CODE END QUADSPI_Init 0 */

	  /* USER CODE BEGIN QUADSPI_Init 1 */

	  /* USER CODE END QUADSPI_Init 1 */
	  /* QUADSPI parameter configuration*/
	  hqspi.Instance = QUADSPI;
	  hqspi.Init.ClockPrescaler = 6;
	  hqspi.Init.FifoThreshold = 4;  // FIFO when 8 more bytes written or read
	  hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
	  hqspi.Init.FlashSize = 25;  	// flash size = 2**(25+1) = 2**26 = 67108864 = 64 Mbytes
	  hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
	  hqspi.Init.ClockMode = QSPI_CLOCK_MODE_0;
	  if (HAL_QSPI_Init(&hqspi) != HAL_OK)
	  {
		Error_Handler();
	  }
	  /* USER CODE BEGIN QUADSPI_Init 2 */

	  /* USER CODE END QUADSPI_Init 2 */

	}

	/**
	* @brief QSPI MSP Initialization
	* This function configures the hardware resources used in this example
	* @param hqspi: QSPI handle pointer
	* @retval None
	*/
	void HAL_QSPI_MspInit(QSPI_HandleTypeDef* hqspi)
	{
	  GPIO_InitTypeDef GPIO_InitStruct = {0};
	  if(hqspi->Instance==QUADSPI)
	  {
	  /* USER CODE BEGIN QUADSPI_MspInit 0 */

	  /* USER CODE END QUADSPI_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_QSPI_CLK_ENABLE();

		__HAL_RCC_GPIOE_CLK_ENABLE();
		/**QUADSPI GPIO Configuration
		PE10     ------> QUADSPI_CLK
		PE11     ------> QUADSPI_NCS
		PE12     ------> QUADSPI_BK1_IO0
		PE13     ------> QUADSPI_BK1_IO1
		PE14     ------> QUADSPI_BK1_IO2
		PE15     ------> QUADSPI_BK1_IO3
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13
							  |GPIO_PIN_14|GPIO_PIN_15;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
		HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	  /* USER CODE BEGIN QUADSPI_MspInit 1 */

	  /* USER CODE END QUADSPI_MspInit 1 */
	  }

	}

	/**
	* @brief QSPI MSP De-Initialization
	* This function freeze the hardware resources used in this example
	* @param hqspi: QSPI handle pointer
	* @retval None
	*/
	void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef* hqspi)
	{
	  if(hqspi->Instance==QUADSPI)
	  {
	  /* USER CODE BEGIN QUADSPI_MspDeInit 0 */

	  /* USER CODE END QUADSPI_MspDeInit 0 */
	    /* Peripheral clock disable */
	    __HAL_RCC_QSPI_CLK_DISABLE();

	    /**QUADSPI GPIO Configuration
	    PE10     ------> QUADSPI_CLK
	    PE11     ------> QUADSPI_NCS
	    PE12     ------> QUADSPI_BK1_IO0
	    PE13     ------> QUADSPI_BK1_IO1
	    PE14     ------> QUADSPI_BK1_IO2
	    PE15     ------> QUADSPI_BK1_IO3
	    */
	    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13
	                          |GPIO_PIN_14|GPIO_PIN_15);

	  /* USER CODE BEGIN QUADSPI_MspDeInit 1 */

	  /* USER CODE END QUADSPI_MspDeInit 1 */
	  }

	}

	/* USER CODE BEGIN 1 */

	/* QUADSPI init function */
	uint8_t CSP_QUADSPI_Init(void) {
		//prepare QSPI peripheral for ST-Link Utility operations
		hqspi.Instance = QUADSPI;
		if (HAL_QSPI_DeInit(&hqspi) != HAL_OK) {
			return HAL_ERROR;
		}

		MX_QUADSPI_Init();

		if (QSPI_ResetChip() != HAL_OK) {
			return HAL_ERROR;
		}

		HAL_Delay(1);

		if (QSPI_AutoPollingMemReady(HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
			return HAL_ERROR;
		}

		if (QSPI_WriteEnable() != HAL_OK) {

			return HAL_ERROR;
		}

		if (QSPI_Configuration() != HAL_OK) {
			return HAL_ERROR;
		}

		return HAL_OK;
	}


	uint8_t CSP_QSPI_Erase_Chip(void) {
		QSPI_CommandTypeDef sCommand;


		if (QSPI_WriteEnable() != HAL_OK) {
			return HAL_ERROR;
		}


		/* Erasing Sequence --------------------------------- */
		sCommand.Instruction = CHIP_ERASE_CMD;
		sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
		sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
		sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
		sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
		sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
		sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
		sCommand.AddressMode = QSPI_ADDRESS_NONE;
		sCommand.Address = 0;
		sCommand.DataMode = QSPI_DATA_NONE;
		sCommand.DummyCycles = 0;


		if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
				!= HAL_OK) {
			return HAL_ERROR;
		}

		if (QSPI_AutoPollingMemReady(MX25R6435F_CHIP_ERASE_MAX_TIME) != HAL_OK) {
					return HAL_ERROR;
				}

		return HAL_OK;
	}

	uint8_t QSPI_AutoPollingMemReady(uint32_t timeout) {

		QSPI_CommandTypeDef sCommand;
		 QSPI_AutoPollingTypeDef sConfig;

		/* Configure automatic polling mode to wait for memory ready ------ */
		sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
		sCommand.Instruction = READ_STATUS_REG_CMD;
		sCommand.AddressMode = QSPI_ADDRESS_NONE;
		sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
		sCommand.DataMode = QSPI_DATA_1_LINE;
		sCommand.DummyCycles = 0;
		sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
		sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
		sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

		sConfig.Match = 0x40;
		sConfig.Mask = 0xFF;
		sConfig.MatchMode = QSPI_MATCH_MODE_AND;
		sConfig.StatusBytesSize = 1;
		sConfig.Interval = 0x10;
		sConfig.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;

		if (HAL_QSPI_AutoPolling(&hqspi, &sCommand, &sConfig, timeout) != HAL_OK) {
			return HAL_ERROR;
		}

		return HAL_OK;
	}

	static uint8_t QSPI_WriteEnable(void) {
		QSPI_CommandTypeDef sCommand;
		QSPI_AutoPollingTypeDef sConfig;

		/* Enable write operations ------------------------------------------ */
		sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
		sCommand.Instruction = WRITE_ENABLE_CMD;
		sCommand.AddressMode = QSPI_ADDRESS_NONE;
		sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
		sCommand.DataMode = QSPI_DATA_NONE;
		sCommand.DummyCycles = 0;
		sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
		sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
		sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

		if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
				!= HAL_OK) {
			return HAL_ERROR;
		}

		/* Configure automatic polling mode to wait for write enabling ---- */
		sConfig.Match = 0x02;
		sConfig.Mask = 0x02;
		sConfig.MatchMode = QSPI_MATCH_MODE_AND;
		sConfig.StatusBytesSize = 1;
		sConfig.Interval = 0x10;
		sConfig.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;

		sCommand.Instruction = READ_STATUS_REG_CMD;
		sCommand.DataMode = QSPI_DATA_1_LINE;
		if (HAL_QSPI_AutoPolling(&hqspi, &sCommand, &sConfig,
		HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
			return HAL_ERROR;
		}

		return HAL_OK;
	}

	uint8_t CSP_QSPI_Read_ID(uint8_t* test_buffer)
	{
		QSPI_CommandTypeDef sCommand;
		/*read status register*/
		sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
		sCommand.Instruction = READ_ID_CMD;
		sCommand.AddressMode = QSPI_ADDRESS_NONE;
		sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
		sCommand.DataMode = QSPI_DATA_1_LINE;
		sCommand.DummyCycles = 0;
		sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
		sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
		sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
		sCommand.NbData = 3;

		if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
			return HAL_ERROR;
		}
		if (HAL_QSPI_Receive(&hqspi, test_buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
			return HAL_ERROR;
		}
		return HAL_OK;
	}

	uint8_t CSP_QSPI_Read_Electonic_ID(uint8_t* test_buffer)
	{
		QSPI_CommandTypeDef sCommand;
		/*read status register*/
		sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
		sCommand.Instruction = READ_ELECTRONIC_ID_CMD;
		sCommand.AddressMode = QSPI_ADDRESS_NONE;
		sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
		sCommand.DataMode = QSPI_DATA_1_LINE;
		sCommand.DummyCycles = 24;
		sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
		sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
		sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
		sCommand.NbData = 1;

		if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
			return HAL_ERROR;
		}
		if (HAL_QSPI_Receive(&hqspi, test_buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
			return HAL_ERROR;
		}
		return HAL_OK;
	}

	uint8_t CSP_QSPI_Read_Manufa_ID(uint8_t* test_buffer)
	{
		QSPI_CommandTypeDef sCommand;
		/*read status register*/
		sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
		sCommand.Instruction = READ_ELEC_MANUFACTURER_DEVICE_ID_CMD;
		sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
		sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
		sCommand.DataMode = QSPI_DATA_1_LINE;
		sCommand.DummyCycles = 16;
		sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
		sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
		sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
		sCommand.NbData = 2;
		sCommand.Address = 0;

		if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
			return HAL_ERROR;
		}
		if (HAL_QSPI_Receive(&hqspi, test_buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
			return HAL_ERROR;
		}
		return HAL_OK;
	}

	uint8_t CSP_QSPI_Read_All_Reg(uint8_t* test_buffer)
	{
		QSPI_CommandTypeDef sCommand;
		/*read status register*/
		sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
		sCommand.Instruction = READ_STATUS_REG_CMD;
		sCommand.AddressMode = QSPI_ADDRESS_NONE;
		sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
		sCommand.DataMode = QSPI_DATA_1_LINE;
		sCommand.DummyCycles = 0;
		sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
		sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
		sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
		sCommand.NbData = 1;

		if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
				!= HAL_OK) {
			return HAL_ERROR;
		}
		if (HAL_QSPI_Receive(&hqspi, test_buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
			return HAL_ERROR;
		}
		/*read configuration register*/
		sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
		sCommand.Instruction = READ_CFG_REG_CMD;
		sCommand.AddressMode = QSPI_ADDRESS_NONE;
		sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
		sCommand.DataMode = QSPI_DATA_1_LINE;
		sCommand.DummyCycles = 0;
		sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
		sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
		sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
		sCommand.NbData = 2;

		if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
				!= HAL_OK) {
			return HAL_ERROR;
		}
		if (HAL_QSPI_Receive(&hqspi, &(test_buffer[1]), HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
			return HAL_ERROR;
		}
		return HAL_OK;
	}

	/*Enable quad mode and set dummy cycles count*/
	uint8_t QSPI_Configuration(void) {

		QSPI_CommandTypeDef sCommand;
		uint8_t test_buffer[4] = { 0 };
		CSP_QSPI_Read_All_Reg(test_buffer);
		/*modify buffer to enable quad mode*/
		test_buffer[0] |= 0x40;

		/*set dummy cycles*/
		test_buffer[1] &= ~0xC0;

		/*enable hight proform*/
		test_buffer[2] |= 0x02;

		sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
		sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
		sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
		sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
		sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
		sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
		sCommand.Instruction = WRITE_STATUS_CFG_REG_CMD;
		sCommand.AddressMode = QSPI_ADDRESS_NONE;
		sCommand.DataMode = QSPI_DATA_1_LINE;
		sCommand.DummyCycles = 0;
		sCommand.NbData = 3;

		if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
				!= HAL_OK) {
			return HAL_ERROR;
		}

		if (HAL_QSPI_Transmit(&hqspi, test_buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
			Error_Handler();
			return HAL_ERROR;
		}
		return HAL_OK;
	}

	uint8_t CSP_QSPI_EraseSector(uint32_t EraseStartAddress, uint32_t EraseEndAddress) {

		QSPI_CommandTypeDef sCommand;

		EraseStartAddress = EraseStartAddress
				- EraseStartAddress % MEMORY_SECTOR_SIZE;

		/* Erasing Sequence -------------------------------------------------- */
		sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
		sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
		sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
		sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
		sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
		sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
		sCommand.Instruction = SECTOR_ERASE_CMD;
		sCommand.AddressMode = QSPI_ADDRESS_1_LINE;

		sCommand.DataMode = QSPI_DATA_NONE;
		sCommand.DummyCycles = 0;

		while (EraseEndAddress >= EraseStartAddress) {
			sCommand.Address = (EraseStartAddress & 0x0FFFFFFF);

			if (QSPI_WriteEnable() != HAL_OK) {
				return HAL_ERROR;
			}

			if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
					!= HAL_OK) {
				return HAL_ERROR;
			}
			EraseStartAddress += MEMORY_SECTOR_SIZE;

			if (QSPI_AutoPollingMemReady(HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
				return HAL_ERROR;
			}
		}

		return HAL_OK;
	}

	uint8_t CSP_QSPI_WriteMemory(uint8_t* buffer, uint32_t address,uint32_t buffer_size) {

		QSPI_CommandTypeDef sCommand;
		uint32_t end_addr, current_size, current_addr;

		/* Calculation of the size between the write address and the end of the page */
		current_addr = 0;


		//
		while (current_addr <= address) {
			current_addr += MEMORY_PAGE_SIZE;
		}
		current_size = current_addr - address;

		/* Check if the size of the data is less than the remaining place in the page */
		if (current_size > buffer_size) {
			current_size = buffer_size;
		}

		/* Initialize the adress variables */
		current_addr = address;
		end_addr = address + buffer_size;

		sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
		sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
		sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
		sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
		sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
		sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
	#ifdef ONE_LINE_WRITE
		// one line
		sCommand.Instruction = PAGE_PROG_CMD;
		sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
		sCommand.DataMode = QSPI_DATA_1_LINE;
	#else
		// four lines
		sCommand.Instruction = QUAD_PAGE_PROG_CMD;
		sCommand.AddressMode = QSPI_ADDRESS_4_LINES;
		sCommand.DataMode = QSPI_DATA_4_LINES;
	#endif
		sCommand.NbData = buffer_size;
		sCommand.Address = address;
		sCommand.DummyCycles = 0;

		/* Perform the write page by page */
		do {
			sCommand.Address = current_addr;
			sCommand.NbData = current_size;

			if (current_size == 0) {
				return HAL_OK;
			}

			/* Enable write operations */
			if (QSPI_WriteEnable() != HAL_OK) {
				return HAL_ERROR;
			}

			/* Configure the command */
			if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
					!= HAL_OK) {

				return HAL_ERROR;
			}

			/* Transmission of the data */
			if (HAL_QSPI_Transmit(&hqspi, buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {

				return HAL_ERROR;
			}

			/* Configure automatic polling mode to wait for end of program */
			if (QSPI_AutoPollingMemReady(HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
				return HAL_ERROR;
			}

			/* Update the address and size variables for next page programming */
			current_addr += current_size;
			buffer += current_size;
			current_size =
					((current_addr + MEMORY_PAGE_SIZE) > end_addr) ?
							(end_addr - current_addr) : MEMORY_PAGE_SIZE;
		} while (current_addr <= end_addr);

		return HAL_OK;

	}

	uint8_t CSP_QSPI_ReadMemory(uint8_t* buffer, uint32_t address,uint32_t buffer_size)
	{
	  QSPI_CommandTypeDef sCommand;
		sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
		sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
		sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
		sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
		sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
		sCommand.AddressMode = QSPI_ADDRESS_4_LINES;
		sCommand.DataMode = QSPI_DATA_4_LINES;
		sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
		sCommand.AlternateBytes = MX25R6435F_ALT_BYTES_PE_MODE;
		sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
		sCommand.NbData = buffer_size;
		sCommand.Address = address;
		sCommand.Instruction = QUAD_INOUT_READ_CMD;
		sCommand.DummyCycles = MX25R6435F_DUMMY_CYCLES_READ_QUAD;


	  /* Configure the command */
	  if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
	    return HAL_ERROR;
	  }

	  /* Transmission of the data */
	  if (HAL_QSPI_Receive(&hqspi, buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
	    return HAL_ERROR;
	  }

	  uint8_t j[1] = {0};
	  sCommand.AlternateBytes = MX25R6435F_ALT_BYTES_NO_PE_MODE;
	  sCommand.NbData = 1;
	  /* Configure the command */
	  if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
	    return HAL_ERROR;
	  }

	  /* Transmission of the data */
	  if (HAL_QSPI_Receive(&hqspi, j, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
	    return HAL_ERROR;
	  }
	  return HAL_OK;
	}

	uint8_t CSP_QSPI_EnableMemoryMappedMode(void) {

		QSPI_CommandTypeDef sCommand;
		QSPI_MemoryMappedTypeDef sMemMappedCfg;

		/* Enable Memory-Mapped mode-------------------------------------------------- */

		sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
		sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
		sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
		sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
		sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
		sCommand.AddressMode = QSPI_ADDRESS_4_LINES;
		sCommand.DataMode = QSPI_DATA_4_LINES;
		sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
		sCommand.AlternateBytes = MX25R6435F_ALT_BYTES_PE_MODE;
		sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
		sCommand.NbData = 0;
		sCommand.Address = 0;
		sCommand.Instruction = QUAD_INOUT_READ_CMD;
		sCommand.DummyCycles = MX25R6435F_DUMMY_CYCLES_READ_QUAD;

		sMemMappedCfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;

		if (HAL_QSPI_MemoryMapped(&hqspi, &sCommand, &sMemMappedCfg) != HAL_OK) {
			return HAL_ERROR;
		}
		return HAL_OK;
	}

	uint8_t QSPI_ResetChip() {
		QSPI_CommandTypeDef sCommand;
		uint32_t temp = 0;
		/* Erasing Sequence -------------------------------------------------- */
		sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
		sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
		sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
		sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
		sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
		sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
		sCommand.Instruction = RESET_ENABLE_CMD;
		sCommand.AddressMode = QSPI_ADDRESS_NONE;
		sCommand.Address = 0;
		sCommand.DataMode = QSPI_DATA_NONE;
		sCommand.DummyCycles = 0;

		if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
				!= HAL_OK) {
			return HAL_ERROR;
		}
		for (temp = 0; temp < 0x2f; temp++) {
			__NOP();
		}

		sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
		sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
		sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
		sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
		sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
		sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
		sCommand.Instruction = RESET_MEMORY_CMD;
		sCommand.AddressMode = QSPI_ADDRESS_NONE;
		sCommand.Address = 0;
		sCommand.DataMode = QSPI_DATA_NONE;
		sCommand.DummyCycles = 0;

		if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
			return HAL_ERROR;
		}
		return HAL_OK;
	}

	/* USER CODE END 1 */
