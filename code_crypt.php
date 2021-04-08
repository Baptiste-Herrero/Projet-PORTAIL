<!-- Clé de cryptage -->
<?php
	$data = 'Je suis le message';
	$key_size = 32; //256 bits
	$iv_size = 16; //128 bits 
	$cypher = 'AES-256-CBC';

	$encryption_key = openssl_random_pseudo_bytes($key_size); //crée une clé aléatoire  d'une taille définie 
	$iv = openssl_random_pseudo_bytes($iv_size); 

	//Encrypt data 
	$encryt_data = openssl_encrypt($data, $cypher, $encryption_key, 0, $iv);// Encrypte toutes les données. 
	echo $data "\n";

	//Décript data 
	$decryt_data = openssl_decrypt($data, $cypher, $encryption_key, 0, $iv);
	echo $encrypt_data;
?>
