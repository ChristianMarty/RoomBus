//---------------------------------------------------------------------------

#ifndef HexFileParserH
#define HexFileParserH

#include <QByteArray>
#include <QString>
#include <QFile>

//---------------------------------------------------------------------------

class targetMemory
{
	public:

        targetMemory(void);

        void setMemorySize(uint32_t adrressOffset, uint32_t size);

        bool pharsHexFile(QString filePath);

        uint32_t getSize(void);
        uint32_t getOffset(void);
        uint32_t getCRC(void);

        void writeSize(uint32_t size);
        void writeCRC(uint32_t crc);

        QByteArray binary;

        uint32_t binaryIndex;

    private:
        uint32_t hexByteCount;
        uint32_t pageWordSize;

        uint32_t minFileAddress;
        uint32_t maxFileAddress;

        uint32_t minAddress;
        uint32_t maxAddress;

        uint32_t hexToInt(QString Hexwert);
};

//---------------------------------------------------------------------------
#endif

