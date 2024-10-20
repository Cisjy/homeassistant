/**
 ******************************************************************************
 * @file    IAP/src/ymodem.c
 * @author  MCD Application Team
 * @version V3.3.0
 * @date    10/15/2010
 * @brief   This file provides all the software functions related to the ymodem
 *          protocol.
 ******************************************************************************
 * @copy
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
 */

/** @addtogroup IAP
 * @{
 */

/* Includes ------------------------------------------------------------------*/
#include "common.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t file_name[FILE_NAME_LENGTH];
uint32_t FlashDestination = ApplicationAddress; /* Flash user program offset */
uint16_t PageSize = PAGE_SIZE;
uint32_t EraseCounter = 0x0;
uint32_t NbrOfPage = 0;
FLASH_Status FLASHStatus = FLASH_COMPLETE;
uint32_t RamSource;
extern uint8_t tab_1024[1024];

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Receive byte from sender
 * @param  c: Character
 * @param  timeout: Timeout
 * @retval 0: Byte received
 *         -1: Timeout
 */
static int32_t Receive_Byte(uint8_t *c, uint32_t timeout)
{
  if (HAL_UART_Receive(&huart1, c, 1, timeout) != HAL_OK)
  {
    return -1;
  }
  return 0;
}

/**
 * @brief  Send a byte
 * @param  c: Character
 * @retval 0: Byte sent
 */
static uint32_t Send_Byte(uint8_t c)
{
  HAL_UART_Transmit(&huart1, &c, 1, 0xFFFF);
  return 0;
}

/**
 * @brief  Receive a packet from sender
 * @param  data
 * @param  length
 * @param  timeout
 *     0: end of transmission
 *    -1: abort by sender
 *    >0: packet length
 * @retval 0: normally return
 *        -1: timeout or packet error
 *         1: abort by user
 */

static int32_t Receive_Packet(uint8_t *data, int32_t *length, uint32_t timeout)
{
  uint16_t packet_size;
  uint8_t c;
  *length = 0;

  if (HAL_UART_Receive(&huart1, &c, 1, timeout) == HAL_TIMEOUT)
  {
    USART2_printf("Receive_Packet timeout\r\n");
    return -1;
  }
  switch (c)
  {
  case SOH:
    packet_size = PACKET_SIZE;
    break;
  case STX:
    packet_size = PACKET_1K_SIZE;
    break;
  case EOT:
    return 0;
  case CA:
    if ((Receive_Byte(&c, timeout) == 0) && (c == CA))
    {
      *length = -1;
      return 0;
    }
    else
    {
      return -1;
    }
  case ABORT1:
  case ABORT2:
    return 1;
  default:
    return -1;
  }
  *data = c;
  HAL_UART_Receive(&huart1, data + 1, packet_size + PACKET_HEADER + PACKET_TRAILER - 1, timeout);

  if (data[PACKET_SEQNO_INDEX] != ((data[PACKET_SEQNO_COMP_INDEX] ^ 0xff) & 0xff))
  {
    return -1;
  }
  *length = packet_size;
  return 0;
}

/**
 * @brief  Receive a file using the ymodem protocol
 * @param  buf: Address of the first byte
 * @retval The size of the file
 */
int32_t Ymodem_Receive(uint8_t *buf)
{
  uint8_t packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD], file_size[FILE_SIZE_LENGTH], *file_ptr, *buf_ptr;
  int32_t i, packet_length, session_done, file_done, packets_received, errors, session_begin, size = 0;
  // my addition
  uint16_t setctor_num = 0;
  uint8_t eot_flag = 0;
  uint32_t total_packet_length = 0;
  /* Initialize FlashDestination variable */
  FlashDestination = ApplicationAddress;

  for (session_done = 0, errors = 0, session_begin = 0;;)
  {
    for (packets_received = 0, file_done = 0, buf_ptr = buf;;)
    {
      switch (Receive_Packet(packet_data, &packet_length, NAK_TIMEOUT))
      {
      case 0:
        errors = 0;
        switch (packet_length)
        {
        /* Abort by sender */
        case -1:
          Send_Byte(ACK);
          return 0;
        /* End of transmission */
        case 0:
          if (eot_flag == 0)
          {
            Send_Byte(NAK);
            eot_flag = 1;
          }
          if (eot_flag == 1)
          {
            Send_Byte(ACK);
            file_done = 1;
            session_done = 1;
            W25QXX_Write(file_name, W25QXX_FLASH_ADDRESS + 5, strlen((const char *)file_name)); // filename
            break;
          }
        /* Normal packet */
        default:
          if ((packet_data[PACKET_SEQNO_INDEX] & 0xff) != (packets_received & 0xff))
          {
            Send_Byte(NAK);
          }
          else
          {
            if (packets_received == 0)
            {
              /* Filename packet */
              if (packet_data[PACKET_HEADER] != 0)
              {
                /* Filename packet has valid data */
                for (i = 0, file_ptr = packet_data + PACKET_HEADER; (*file_ptr != 0) && (i < FILE_NAME_LENGTH);)
                {
                  file_name[i++] = *file_ptr++;
                }
                file_name[i++] = '\0';
                for (i = 0, file_ptr++; (*file_ptr != ' ') && (i < FILE_SIZE_LENGTH);)
                {
                  file_size[i++] = *file_ptr++;
                }
                file_size[i++] = '\0';
                Str2Int(file_size, &size);

                /*前一个SECTOR的地址 放置文件大小*/
                for (setctor_num = 0; setctor_num < (size / W25QXX_SECTOR_SIZE) + 2; setctor_num++)
                {
                  W25QXX_Erase_Sector(W25QXX_FLASH_ADDRESS + (setctor_num * W25QXX_SECTOR_SIZE));
                }
                W25QXX_Erase_Sector(W25QXX_FLASH_ADDRESS);
                uint8_t *psize = (uint8_t *)&size;
                W25QXX_Write(psize, W25QXX_FLASH_ADDRESS, 4);
                Send_Byte(ACK);
                Send_Byte(CRC16);
              }
              /* Filename packet is empty, end session */
              else
              {
                Send_Byte(ACK);
                file_done = 1;
                session_done = 1;
                break;
              }
            }
            /* Data packet */
            else
            {
              memcpy(buf_ptr, packet_data + PACKET_HEADER, packet_length);
              RamSource = (uint32_t)buf;

              W25QXX_Write(packet_data + PACKET_HEADER, W25QXX_YMODEM_OFFSET + total_packet_length, packet_length);
              // USART2_printf("total_packet_length = %d,w25qxx OFFSET + total_packet_length = 0x%x\r\n", total_packet_length, W25QXX_YMODEM_OFFSET + total_packet_length);
              total_packet_length = total_packet_length + packet_length;
              Send_Byte(ACK);
            }
            packets_received++;
            session_begin = 1;
          }
        }
        break;
      case 1:
        Send_Byte(CA);
        Send_Byte(CA);
        return -3;
      default:
        if (session_begin > 0)
        {
          errors++;
        }
        if (errors > MAX_ERRORS)
        {
          Send_Byte(CA);
          Send_Byte(CA);
          return 0;
        }
        Send_Byte(CRC16);
        if (oled_comfirm_flag & 0x04)
        {
          oled_comfirm_flag &= 0x00;
          return 0;
        }
        break;
      }
      if (file_done != 0)
      {
        break;
      }
    }

    if (session_done != 0)
    {
      break;
    }
  }
  return (int32_t)size;
}

/**
 * @brief  check response using the ymodem protocol
 * @param  buf: Address of the first byte
 * @retval The size of the file
 */
int32_t Ymodem_CheckResponse(uint8_t c)
{
  return 0;
}

/**
 * @brief  Prepare the first block
 * @param  timeout
 *     0: end of transmission
 */
void Ymodem_PrepareIntialPacket(uint8_t *data, const uint8_t *fileName, uint32_t *length)
{
  uint16_t i, j;
  uint8_t file_ptr[10];

  /* Make first three packet */
  data[0] = SOH;
  data[1] = 0x00;
  data[2] = 0xff;

  /* Filename packet has valid data */
  for (i = 0; (fileName[i] != '\0') && (i < FILE_NAME_LENGTH); i++)
  {
    data[i + PACKET_HEADER] = fileName[i];
  }

  data[i + PACKET_HEADER] = 0x00;

  Int2Str(file_ptr, *length);
  for (j = 0, i = i + PACKET_HEADER + 1; file_ptr[j] != '\0';)
  {
    data[i++] = file_ptr[j++];
  }

  for (j = i; j < PACKET_SIZE + PACKET_HEADER; j++)
  {
    data[j] = 0;
  }
}

/**
 * @brief  Prepare the data packet
 * @param  timeout
 *     0: end of transmission
 */
void Ymodem_PreparePacket(uint8_t *SourceBuf, uint8_t *data, uint8_t pktNo, uint32_t sizeBlk)
{
  uint16_t i, size, packetSize;
  uint8_t *file_ptr;

  /* Make first three packet */
  packetSize = sizeBlk >= PACKET_1K_SIZE ? PACKET_1K_SIZE : PACKET_SIZE;
  size = sizeBlk < packetSize ? sizeBlk : packetSize;
  if (packetSize == PACKET_1K_SIZE)
  {
    data[0] = STX;
  }
  else
  {
    data[0] = SOH;
  }
  data[1] = pktNo;
  data[2] = (~pktNo);
  file_ptr = SourceBuf;

  /* Filename packet has valid data */
  for (i = PACKET_HEADER; i < size + PACKET_HEADER; i++)
  {
    data[i] = *file_ptr++;
  }
  if (size <= packetSize)
  {
    for (i = size + PACKET_HEADER; i < packetSize + PACKET_HEADER; i++)
    {
      data[i] = 0x1A; /* EOF (0x1A) or 0x00 */
    }
  }
}

/**
 * @brief  Update CRC16 for input byte
 * @param  CRC input value
 * @param  input byte
 * @retval None
 */
uint16_t UpdateCRC16(uint16_t crcIn, uint8_t byte)
{
  uint32_t crc = crcIn;
  uint32_t in = byte | 0x100;
  do
  {
    crc <<= 1;
    in <<= 1;
    if (in & 0x100)
      ++crc;
    if (crc & 0x10000)
      crc ^= 0x1021;
  } while (!(in & 0x10000));
  return crc & 0xffffu;
}

/**
 * @brief  Cal CRC16 for YModem Packet
 * @param  data
 * @param  length
 * @retval None
 */
uint16_t Cal_CRC16(const uint8_t *data, uint32_t size)
{
  uint32_t crc = 0;
  const uint8_t *dataEnd = data + size;
  while (data < dataEnd)
    crc = UpdateCRC16(crc, *data++);

  crc = UpdateCRC16(crc, 0);
  crc = UpdateCRC16(crc, 0);
  return crc & 0xffffu;
}

/**
 * @brief  Cal Check sum for YModem Packet
 * @param  data
 * @param  length
 * @retval None
 */
uint8_t CalChecksum(const uint8_t *data, uint32_t size)
{
  uint32_t sum = 0;
  const uint8_t *dataEnd = data + size;
  while (data < dataEnd)
    sum += *data++;
  return sum & 0xffu;
}

/**
 * @brief  Transmit a data packet using the ymodem protocol
 * @param  data
 * @param  length
 * @retval None
 */
void Ymodem_SendPacket(uint8_t *data, uint16_t length)
{
  uint16_t i;
  i = 0;
  while (i < length)
  {
    Send_Byte(data[i]);
    i++;
  }
}

/**
 * @brief  Transmit a file using the ymodem protocol
 * @param  buf: Address of the first byte
 * @retval The size of the file
 */
uint8_t Ymodem_Transmit(uint8_t *buf, const uint8_t *sendFileName, uint32_t sizeFile)
{

  uint8_t packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD];
  uint8_t FileName[FILE_NAME_LENGTH];
  uint8_t *buf_ptr, tempCheckSum;
  uint16_t tempCRC, blkNumber;
  uint8_t receivedC[2], CRC16_F = 0, i;
  uint32_t errors, ackReceived, size = 0, pktSize;

  errors = 0;
  ackReceived = 0;
  for (i = 0; i < (FILE_NAME_LENGTH - 1); i++)
  {
    FileName[i] = sendFileName[i];
  }
  CRC16_F = 1;

  /* Prepare first block */
  Ymodem_PrepareIntialPacket(&packet_data[0], FileName, &sizeFile);

  do
  {
    /* Send Packet */
    Ymodem_SendPacket(packet_data, PACKET_SIZE + PACKET_HEADER);
    /* Send CRC or Check Sum based on CRC16_F */
    if (CRC16_F)
    {
      tempCRC = Cal_CRC16(&packet_data[3], PACKET_SIZE);
      Send_Byte(tempCRC >> 8);
      Send_Byte(tempCRC & 0xFF);
    }
    else
    {
      tempCheckSum = CalChecksum(&packet_data[3], PACKET_SIZE);
      Send_Byte(tempCheckSum);
    }

    /* Wait for Ack and 'C' */
    if (Receive_Byte(&receivedC[0], 10000) == 0)
    {
      if (receivedC[0] == ACK)
      {
        /* Packet transfered correctly */
        ackReceived = 1;
      }
    }
    else
    {
      errors++;
    }
  } while (!ackReceived && (errors < 0x0A));

  if (errors >= 0x0A)
  {
    return errors;
  }
  buf_ptr = buf;
  size = sizeFile;
  blkNumber = 0x01;
  /* Here 1024 bytes package is used to send the packets */

  /* Resend packet if NAK  for a count of 10 else end of commuincation */
  while (size)
  {
    /* Prepare next packet */
    Ymodem_PreparePacket(buf_ptr, &packet_data[0], blkNumber, size);
    ackReceived = 0;
    receivedC[0] = 0;
    errors = 0;
    do
    {
      /* Send next packet */
      if (size >= PACKET_1K_SIZE)
      {
        pktSize = PACKET_1K_SIZE;
      }
      else
      {
        pktSize = PACKET_SIZE;
      }
      Ymodem_SendPacket(packet_data, pktSize + PACKET_HEADER);
      /* Send CRC or Check Sum based on CRC16_F */
      /* Send CRC or Check Sum based on CRC16_F */
      if (CRC16_F)
      {
        tempCRC = Cal_CRC16(&packet_data[3], pktSize);
        Send_Byte(tempCRC >> 8);
        Send_Byte(tempCRC & 0xFF);
      }
      else
      {
        tempCheckSum = CalChecksum(&packet_data[3], pktSize);
        Send_Byte(tempCheckSum);
      }

      /* Wait for Ack */
      if ((Receive_Byte(&receivedC[0], 100000) == 0) && (receivedC[0] == ACK))
      {
        ackReceived = 1;
        if (size > pktSize)
        {
          buf_ptr += pktSize;
          size -= pktSize;
          if (blkNumber == (FLASH_IMAGE_SIZE / 1024))
          {
            return 0xFF; /*  error */
          }
          else
          {
            blkNumber++;
          }
        }
        else
        {
          buf_ptr += pktSize;
          size = 0;
        }
      }
      else
      {
        errors++;
      }
    } while (!ackReceived && (errors < 0x0A));
    /* Resend packet if NAK  for a count of 10 else end of commuincation */

    if (errors >= 0x0A)
    {
      return errors;
    }
  }
  ackReceived = 0;
  receivedC[0] = 0x00;
  errors = 0;
  do
  {
    Send_Byte(EOT);
    /* Send (EOT); */
    /* Wait for Ack */
    if ((Receive_Byte(&receivedC[0], 10000) == 0) && receivedC[0] == ACK)
    {
      ackReceived = 1;
    }
    else
    {
      errors++;
    }
  } while (!ackReceived && (errors < 0x0A));

  if (errors >= 0x0A)
  {
    return errors;
  }

  /* Last packet preparation */
  ackReceived = 0;
  receivedC[0] = 0x00;
  errors = 0;

  packet_data[0] = SOH;
  packet_data[1] = 0;
  packet_data[2] = 0xFF;

  for (i = PACKET_HEADER; i < (PACKET_SIZE + PACKET_HEADER); i++)
  {
    packet_data[i] = 0x00;
  }

  do
  {
    /* Send Packet */
    Ymodem_SendPacket(packet_data, PACKET_SIZE + PACKET_HEADER);
    /* Send CRC or Check Sum based on CRC16_F */
    tempCRC = Cal_CRC16(&packet_data[3], PACKET_SIZE);
    Send_Byte(tempCRC >> 8);
    Send_Byte(tempCRC & 0xFF);

    /* Wait for Ack and 'C' */
    if (Receive_Byte(&receivedC[0], 10000) == 0)
    {
      if (receivedC[0] == ACK)
      {
        /* Packet transfered correctly */
        ackReceived = 1;
      }
    }
    else
    {
      errors++;
    }

  } while (!ackReceived && (errors < 0x0A));
  /* Resend packet if NAK  for a count of 10  else end of commuincation */
  if (errors >= 0x0A)
  {
    return errors;
  }

  do
  {
    Send_Byte(EOT);
    /* Send (EOT); */
    /* Wait for Ack */
    if ((Receive_Byte(&receivedC[0], 10000) == 0) && receivedC[0] == ACK)
    {
      ackReceived = 1;
    }
    else
    {
      errors++;
    }
  } while (!ackReceived && (errors < 0x0A));

  if (errors >= 0x0A)
  {
    return errors;
  }
  return 0; /* file trasmitted successfully */
}

/**
 * @}
 */

/*******************(C)COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
