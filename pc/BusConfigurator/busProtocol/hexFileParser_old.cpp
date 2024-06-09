//---------------------------------------------------------------------------

#include "hexFileParser_old.h"
#include "../../QCodebase/crc.h"

#define DataRecordStr "00"
#define EndOfFileRecordStr "01"
#define ExtendedSegmentAddressRecord "02"
#define ExtendedLinearAddressRecordStr "04"
//---------------------------------------------------------------------------
void targetMemory::writeSize(uint32_t size)
{
    QByteArray sizeArray;
    sizeArray.append((char)(size&0xFF));
    sizeArray.append((char)((size>>8)&0xFF));
    sizeArray.append((char)((size>>16)&0xFF));
    sizeArray.append((char)((size>>24)&0xFF));

    binary.replace(4,4,sizeArray);
}

void targetMemory::writeCRC(uint32_t crc)
{
    QByteArray sizeArray;
    sizeArray.append((char)(crc&0xFF));
    sizeArray.append((char)((crc>>8)&0xFF));
    sizeArray.append((char)((crc>>16)&0xFF));
    sizeArray.append((char)((crc>>24)&0xFF));

    binary.replace(0,4,sizeArray);
}

targetMemory::targetMemory(void)
{
    minAddress = 0;
    maxAddress = 0xFFFFFFFF;
}

void targetMemory::setMemorySize(uint32_t adrressOffset, uint32_t size)
{
    minAddress = adrressOffset;
    maxAddress = adrressOffset+size;
}

uint32_t targetMemory::getSize(void)
{
    return binary.size();
}

uint32_t targetMemory::getOffset(void)
{
    return minFileAddress;
}

uint32_t targetMemory::getCRC(void)
{
    //Poly =  0x04C11DB7;
   /* uint32_t crc = 0xffffffff;

    for(uint32_t i = 4; i < getSize() ; i++)
    {
        crc = CRC::crcTable[ (crc ^ (uint32_t)binary.at(i)) & 0xFF ] ^ (crc >> 8);
    }*/
    return QuCLib::Crc::crc32(binary.mid(4));
}

//---------------------------------------------------------------------------
bool targetMemory::pharsHexFile(QString filePath)
{
    binary.clear();

    minFileAddress = 0xFFFFFFFF;
    maxFileAddress = 0;

    QString line;
    QFile hexFile(filePath);
    hexFile.open(QIODevice::ReadOnly);

    if(hexFile.isOpen())
	{
        uint32_t high16BitAddress = 0;

        // First find size and min / max address
        while(!hexFile.atEnd())
		{
           line = hexFile.readLine();
           if(line.at(0) == ':')
			{
                QString typStr = line.mid(7,2);

				if(typStr == DataRecordStr)
				{
                    uint32_t lineByteCount = line.mid(1,2).toUInt(nullptr,16);
                    uint32_t lineAddress = line.mid(3,4).toUInt(nullptr,16);

                    if(minFileAddress > (high16BitAddress+lineAddress)) minFileAddress = (high16BitAddress+lineAddress);
                    if(maxFileAddress < (high16BitAddress+lineAddress + lineByteCount)) maxFileAddress = (high16BitAddress+lineAddress + lineByteCount-1);

				}
                else if(typStr == ExtendedSegmentAddressRecord)
                {
                    high16BitAddress = (line.mid(9,4).toUInt(nullptr,16))<<4;
                }
				else if(typStr == ExtendedLinearAddressRecordStr)
				{
                     high16BitAddress = line.mid(9,4).toUInt(nullptr,16)<<16;
                }
                else if(typStr == EndOfFileRecordStr)
                {
                    break;
                }
			}
        }

        for(uint64_t i = 0; i<(maxFileAddress - minFileAddress);i++)
        {
            binary.append((char)0xFF);
        }


        hexFile.seek(0);
        high16BitAddress = 0;

        while(!hexFile.atEnd())
		{
            line = hexFile.readLine();
            if(line.at(0) == ':')
             {
                QString typStr = line.mid(7,2);

                if(typStr == DataRecordStr)
				{
                    uint32_t lineByteCount = line.mid(1,2).toUInt(nullptr,16);
                    uint32_t lineAddress = line.mid(3,4).toUInt(nullptr,16);

                    for(uint32_t i = 0; i < lineByteCount; i++)
					{
                        uint8_t byte = line.mid((9+(i*2)),2).toUInt(nullptr,16);
                        uint32_t byteAddress = (high16BitAddress + lineAddress + i);

                        if((minAddress <= byteAddress) && (maxAddress > byteAddress))
                        {
                            uint32_t byteIndex = byteAddress - minFileAddress;
                            QByteArray temp;
                            temp.append(byte);
                            binary.replace(byteIndex,1,temp);
                        }
					}
				}
                else if(typStr == ExtendedSegmentAddressRecord)
                {
                    high16BitAddress = (line.mid(9,4).toUInt(nullptr,16))<<4;
                }
                else if(typStr == ExtendedLinearAddressRecordStr)
                {
                    high16BitAddress = line.mid(9,4).toUInt(nullptr,16)<<16;
                }
                else if(typStr == EndOfFileRecordStr)
                {
                    break;
                }
			}
			else
			{
                break;
			}
        }
        hexFile.close();

        return true;
    }
    else
    {
        return false;
    }
}
//---------------------------------------------------------------------------
