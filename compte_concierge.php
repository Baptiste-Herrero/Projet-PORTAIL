<!--====================================================================================
	fichier: compte_concierge.php
	base de donnée: base_distante
	auteur : Herrero Baptiste
	date : 05/03/2021
	dernière mise à jour : 13/03/2021
	rôle : permet d'encrypter un utilisateur et sont mot de passe et de l'inssérer dans 
	       une base de donnée
	projet : PORTAIL
	résultat dans : la table Identification de la base distante 
=======================================================================================-->

<?php
	//Connection BDD:
	$bdd = new PDO('mysql:host=localhost;dbname=base_distante;charset=utf8','Snir', 'Snir2020*');

	//Info pour le concierge en claire:
	$user = 'Concierge';
	$passwd = 'Concierge2020*';

	//Clé d'encryptage:
	$encryption_key = 's5u8x/A?D(G+KbPeShVmYq3t6w9z$B&E';
	$iv = 'RfUjWnZr4u7x!A%D'; 
	$cypher = 'AES-256-CBC';

	//Encryption des donnée:
	$encrytID = openssl_encrypt($user, $cypher, $encryption_key, 0, $iv);
	$encryptMDP = openssl_encrypt($passwd, $cypher, $encryption_key, 0, $iv);

	//Envoie des données crypter dans la base de donnée 
	$req=$bdd->prepare("INSERT INTO Identification(ID, MDP) VALUES(:ID, :MDP)");
	$req->execute(array('ID'=>$encrytID, 'MDP'=>$encryptMDP));
?> 