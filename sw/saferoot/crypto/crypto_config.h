// SafeRoot crypto configuration
// SPHINCS+ is optional — loaded from Flash when needed
#ifndef SAFEROOT_CRYPTO_CONFIG_H_
#define SAFEROOT_CRYPTO_CONFIG_H_

// Set to 1 to include SPHINCS+ in ROM (adds ~3.3KB)
// Set to 0 to load from Flash at runtime (saves ROM space)
#ifndef SAFEROOT_ENABLE_SPHINCS
#define SAFEROOT_ENABLE_SPHINCS  0
#endif

// ECDSA P-256 always included (needed for boot verify)
#define SAFEROOT_ENABLE_ECDSA    1

#endif
