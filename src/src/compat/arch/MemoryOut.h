#pragma once

/*host aplikacija tj dtwin zahtjeva da nas plugin implementira pravila od natID/dTwin,
zato moramo imati sljedece metode:

void show(...) override final
td::String getMenuName() const override final
arch::MemoryOut* getArchive(...) override final
MemoryArchiveContainer& getArchives() override final
td::String getOutFileName() const override final
size_t getMaxSupportedArchiveParts() const override final
ModelType getModelType() const override final

*/

#include <td/Types.h>
//SDK je Software Development  Kit (natID.SDK)
//API je Application Programming Interface

namespace arch //grupise klase/funkcije i sprjecava sudar imena, MemoryOut sada pripada oblasti arch

{
    //MemoryOut sluzi kao izlazna memorijska arhiva, alternativa upisa u .dmodl zbog kompatibilnosti sa sc::IPlugin (iz natID.SDK) API-jem
class MemoryOut//pravimo kao fallback ako ne pronadje SDK header

{
public:
    enum class PageSize : td::BYTE { Small = 0, Normal };//cuva kompatibilan API sa pravim MemoryOut
    
    static MemoryOut* allocate(PageSize pageSize = PageSize::Normal)
    {
        (void) pageSize;//ignorisi pageSize
        return new MemoryOut();
    }
    
    bool open(const char* pFileName = nullptr)
    {
        (void) pFileName; //ignorisi pFileName
        return true;
    }
    
    void release()
    {
        delete this;//oslobadjanje memorije
    }
    
    void put(const char* pStr)
    {
        (void) pStr;//utisava warning
    }
    
    void put(const char* pStr, td::UINT4 len)
    {
        (void) pStr;//isto kao prethodno ali kada imamo len
        (void) len;
    }
};
}
