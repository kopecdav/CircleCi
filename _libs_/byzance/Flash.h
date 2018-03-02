#ifndef FLASH_H
#define FLASH_H

#include "mbed.h"

#include "byzance_debug.h"
#include "ByzanceLogger.h"
#include "ExtMem.h"

/**
* \class Flash
* \brief brief TO DO
*
* Description TO DO
*/
class Flash {

	// list of classes that can access protected functions
	friend class Byzance;

	public:

		/** Read data from external flash memory
		 *
		 * \param offset	offset from begin
		 * \param data		data to be read
		 * \param size		size of data in bytes
		 * \return positive	number of read bytes
		 * \return negative	error code
		 */
		static int read(uint32_t offset, void *data, uint32_t size);

		/** Write data to external flash memory
		 *
		 * \param	offset	offset from begin
		 * \param	data	data to be written
		 * \param	size	size of data in bytes
		 * \return	positive	number of written bytes
		 * \return	negative	error code
		 */
		static int write(uint32_t offset, void *data, uint32_t size);

		/** Size of user part in external flash memory
		 *
		 * \param	none
		 * \return	size of user part in bytes
		 */
		static int size(void);

		/** Ready
		 *
		 * \param	none
		 * \return	true/false
		 */
		static bool ready(void);

	protected:

		// can be called only from friend class -> Byzance class
		static void init(ExtMem* extmem);

		/** Prevent user to write outside of the user area
		 *
		 * \param	addr	address
		 * \return	true/false
		 */
		static bool addr_ok(uint32_t offset);

	private:

		static ExtMem*	_extmem;
		static bool		_initialised;

		/**
		 * Private constructor and destructor
		 */
		Flash() {};
		~Flash() {};
};

#endif /* CONSOLE_H */
