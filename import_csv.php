<!--====================================================================================
	fichier: import_csv.php
	base de donnée : base_distante
	auteur : Herrero Baptiste
	date : 04/03/2021	
	dernière mise à jour : 13/03/2021
	rôle : permet d'encrypter un fichier CSV et de l'inssérer dans une base de donnée
	projet : PORTAIL
	résultat dans : la table Information de la base distante 
=======================================================================================-->

<?php
    //Connection à la base de donnée 
	$bdd = new PDO('mysql:host=localhost;dbname=base_distante;charset=utf8','root', '');
	//l'accée du fichier sur le serveur 
	$fichier = fopen("donnee.csv", "r");

	while (!feof($fichier)) {
		//on récupére toute la ligne
		$uneLigne = fgets($fichier);
		//on met dans un tableau les différentes valeur trouver (séparer par un ;)
		$tableauValeurs = explode(';', $uneLigne);

		//Création de variable pour l'encryptage
		$plaque = $tableauValeurs[0];
		$prenom = $tableauValeurs[1];
		$nom = $tableauValeurs[2];

		//Encryptage des données:
		//Clé d'encryptage:
		$encryption_key = 's5u8x/A?D(G+KbPeShVmYq3t6w9z$B&E';
	    $iv = 'RfUjWnZr4u7x!A%D'; 
	    $cypher = 'AES-256-CBC';

	    //Fonction d'encryption: 
	    $encrytnom = openssl_encrypt($nom, $cypher, $encryption_key, 0, $iv);
	    $encryptprenom = openssl_encrypt($prenom, $cypher, $encryption_key, 0, $iv);
	    $encrytplaque = openssl_encrypt($plaque, $cypher, $encryption_key, 0, $iv);

	    //on crée la requete SQL pour inserer les données 
		//$sql ="INSERT INTO information(plaque, prenom, nom) VALUES (".$tableauValeurs[0].",".$tableauValeurs[1].",".$tableauValeurs[2].");";
		$req=$bdd->prepare("INSERT INTO information(plaque, prenom, nom) VALUES(:plaque, :prenom, :nom)");
		$req->execute(array('plaque'=>$encrytplaque, 'prenom'=>$encryptprenom, 'nom'=>$encrytnom));
		//echo $sql; //ligne de debug 
		
	}
?>