<!--====================================================================================
	fichier: synchro.php
	bases de données : base_distante vers base_local
	auteur : Herrero Baptiste
	date : 12/03/2021
	dernière mise à jour : 18/03/2021
	rôle : Envoie les donnée de la base_distante vers la base_local
	projet : PORTAIL
	résultat dans : la table Information de la base distante 
=======================================================================================-->

<?php
    //Connection à la base de donnée: base_distante
    try{
        $bdd = new PDO('mysql:host=localhost;dbname=base_distante;charset=utf8','Snir', 'Snir2020*');
    } 
    catch(Exception $e) {         
        die('Erreur : '.$e->getMessage()); 
    }

    //Connection à la base de donnée: base_local
    try{
        $bdd2 = new PDO('mysql:host=172.20.233.59:3306 ;dbname=base_local;charset=utf8','Admin', 'Admin2020*'); 
    }
    catch(Exception $e) {         
        die('Erreur : '.$e->getMessage()); 
    } 
    //penser a demander comment mettre une IP dans cette ligne.  

    //Récupére toutes les informations de la base distante 
    $req = $bdd->query("SELECT * FROM Information");
    //Debug pour l'afficher sur une page web. 
    while ($data = $req->fetch()) {
        print_r ($data);
        echo '<br>';
        //Envoie des donnée vers la base de donnée local
        $req2 = $bdd2->prepare("INSERT INTO Information(plaque, prenom, nom) VALUES (:plaque, :prenom, :nom)");
    	$req2->execute(array('plaque'=>$data[1], 'prenom'=>$data[2] , 'nom'=>$data[3]));
    } 
?>
