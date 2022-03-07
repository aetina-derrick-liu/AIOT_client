# Generate RSA and AES Keys Using OpenSSL
## Generate a RSA keypair with a 2048 bit private key
Execute command:

    $ openssl genpkey -out private_key.pem -algorithm RSA -pkeyopt rsa_keygen_bits:2048

## Extracting the public key from an RSA keypair
Execute command:

    $ openssl rsa -pubout -in private_key.pem -out public_key.pem

## Genrate an AES key/iv pari with 256-bit cipher
Execute command:

    $ openssl enc -nosalt -aes-256-cbc -P