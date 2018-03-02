#include "IntMem.h"
    
// TODO: add bootloader - interface bootloader_print
REGISTER_DEBUG(IntMem);

IntMem_t IntMem::init(void) {
	__TRACE("init begin\n");

	HAL_FLASH_Unlock();

	// Clear pending flags (if any)
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

	__TRACE("init end\n");

	return INTMEM_OK;

}

IntMem_t IntMem::erase(uint32_t StartAddress, uint32_t EndAddress) {

	__TRACE("erase begin\n");

	// there are sectors in Cortex M4 and pages in Cortex M0

	uint32_t start;
	uint32_t error;
	FLASH_EraseInitTypeDef pEraseInit;

	uint32_t num = 0;

	// Unlock the Flash to enable the flash control register access
	IntMem::init();

	// Get the sector where starts the user flash area
	start = IntMem::get_sector(StartAddress);
	num = (IntMem::get_sector(EndAddress) - start)+1;

	pEraseInit.TypeErase = TYPEERASE_SECTORS;
	pEraseInit.Sector = start;
	pEraseInit.NbSectors = num;
	pEraseInit.VoltageRange = VOLTAGE_RANGE_3;
    
	if (HAL_FLASHEx_Erase(&pEraseInit, &error) != HAL_OK)
	{
		// error occurred while page erase
		__ERROR("erase error\n");
		return INTMEM_ERASE_ERROR;
	}

	__TRACE("erase end\n");

	return INTMEM_OK;
}

IntMem_t IntMem::read_data(uint32_t address, uint32_t offset, void *data, uint32_t size){

	__TRACE("read_data begin\n");

	// TODO: přidat podmínku, která kontroluje, jestli čtu v rámci FLASH paměti
	// TODO: přidat podmínku, co kontroluje návratovou hodnotu z memcpy a případně vrátí error
	// TODO: skutecne tu musi byt memcpy? Nestaci udelat pointer, pres ktery to pujde ven?
	// ByzanceLogger::log("Ctu %d bytu od adresy 0x%08X\n", size, part+offset);
	memcpy((uint8_t*)data, (void*)(address+offset), size);

	__TRACE("read_data end\n");

	return INTMEM_OK;
}

uint32_t IntMem::get_sector(uint32_t address){

	__TRACE("get_sector: begin\n");

	uint32_t map[FLASH_SECTORS];

	get_map(map);

	uint32_t sector = 0;

	for(uint32_t i = 0; i<(FLASH_SECTORS-1); i++){
		if((address>=map[i])&&(address<map[i+1])){
			sector = i;
			break;
		}
	}

	// pro Cortex M0 tato funkce vůbec není potřeba
	// protože se používá akorát při mazání a M0 maže po stránkách (nikoliv sektorech)
	// a parametrem není číslo sektoru, ale přímo adresa stránky

	__DEBUG("get sector: index = %d\n", sector);

	__TRACE("get_sector: end\n");

	return sector;
}

IntMem_t IntMem::write_data(const uint32_t address, const uint32_t offset, const void *data, const uint32_t size, const bool erase){

	__TRACE("write_data begin\n");

	static FLASH_EraseInitTypeDef EraseInitStruct;

	// zero size is invalid
	if(size == 0){
		__ERROR("write_data error, size is 0\n");
		return INTMEM_BAD_ADDR;
	}

	// data are bigger than intmem
	if (size > INTMEM_SIZEOF){
		__ERROR("write_data error, data are big\n");
		return INTMEM_DATA_BIG;
	}

	// data will overflow intmem during write
	if ((address + offset + size) > INTMEM_BEGIN + INTMEM_SIZEOF){
		__ERROR("write_data error, data overflow flash\n");
		return INTMEM_DATA_BIG;
	}

	// zacatek dat je jinde
	if(address + offset < INTMEM_BEGIN){
		__ERROR("write_data error, data begin is invalid\n");
		return INTMEM_BAD_ADDR;
	}

	// odemče FLASH
	HAL_FLASH_Unlock();
	// todo: kontrolovat navratovou hodnotu

	uint32_t i = 0;
	uint32_t zacatek_zapisu;
	zacatek_zapisu = address+offset;

#warning "Mod automazani zatim neni pro IODAG2 implementovany."

	{

		// TODO: následující cyklus zapisuje vždy po násobku 4 bytů
		// pokud se budou zapisovat nějaká data, která nejsou o násobku 4 bytů
		// dojde k zapsání víc dat, než je třeba (o 0-3 byty)
		// to by mohlo případně poškodit data, která následují a neměly by být přepsány

		// problém se bude odstraňovat dost netriviálně,
		// protože funkce HAL_FLASH_Program funguje jenom s TYPEPROGRAM_WORD délkou :-(

		// todo: pridat debug warning hlasku, pokud nastala vyse popsana situace

		for (i = 0; i < size/4; i++){

			// tady by to asi slo bez memcpy
			// jenom pomoci pointeru
			uint32_t tmp32 = 0;
			memcpy(&tmp32, (char*)data+i*4, 4);

			// Device voltage range supposed to be [2.7V to 3.6V], the operation will be done by word
			if (HAL_FLASH_Program(TYPEPROGRAM_WORD, zacatek_zapisu, tmp32) == HAL_OK){
				// Increment FLASH destination address
				zacatek_zapisu += 4;
			}else{
				// Error occurred while writing data in Flash memory
				return INTMEM_WRITE_ERROR;
			}
		}
	}

	//HAL_FLASH_Lock();
	return INTMEM_OK;
}
