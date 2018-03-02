#ifndef BYZANCE_EXTERN_DECLARATION_H
#define BYZANCE_EXTERN_DECLARATION_H

/*
 * Vznikal problém v nekonzistentnosti BUILD ID.
 * Pokud se nějaké knihovny zkompilovaly s nějakým BUILD ID
 * a pak se někde lokálně změnily a znovu se zkompilovala jenom část změněných souborů,
 * tak v určité části kódů bylo určité BUILD ID a v jiné části kódů je jiné BUILD ID, což je nežádoucí.
 *
 * Tímto externem řekneme kompilátoru, aby se díval všude možně do kódů (když se změní build id),
 * aby to bylo všude konzistentní.
 *
 * Fíčura implementovaná @Viktor a @David
 */

extern const char 	*get_byzance_build_id(void);

#endif /* BYZANCE_EXTERN_DECLARATION_H */
