/*
    module   : sqlrequest.cpp
    projet   : connecteur objet mysql

    version  : 1.0
    auteur   : profs BTS SNIR
    creation : 18/02/19
    modif    :
*/

// SPDX-License-Identifier: GPL-3.0-or-later

#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cassert>

#include "sqlrequest.h"

using namespace std;
//---------------------------------------------------------------------------------
//menu Project/Build options...
//onglet #defines
//ajouter BUILD=1

SQLrequest::SQLrequest(const string& host, const string& user, const string& passwd, const string& db, unsigned int port)
{
    /** constructeur */
    this->host   = host;
    this->user   = user;
    this->passwd = passwd;
    this->db     = db;
    this->port   = port;
    this->valid  = (mysql_init(&mysql) != nullptr);

    if ( !this->valid )
        last_error << "Init failed!" << endl;
}

SQLrequest::~SQLrequest(void)
{
    close();
}

bool SQLrequest::connect(void)
{
    /**
    connexion à la base de données

    @retval true si connexion réussie
    @retval false si échec de connexion
    */
    if ( !isValid() )
        return false;

    if ( mysql_real_connect(&mysql, host.data(), user.data(), passwd.data(), db.data(), port, NULL, 0) == nullptr )
    {
        last_error << "Can't connect to database: " << mysql_error(&mysql) << endl;
        return false;
    }
    return true;
}

bool SQLrequest::close(void)
{
    /** ferme la connexion au serveur (appelée automatiquement par le destructeur)

    @retval true si une connexion au serveur était ouverte
    @retval false sinon
    */
    if ( isValid() )
    {
        mysql_close(&mysql);
        return true;
    }
    return false;
}

string SQLrequest::get_last_error(void) const
{
    /** renvoie la dernière erreur sql */
    return last_error.str();
}

string SQLrequest::get_last_request(void) const
{
    /** renvoie la dernière requête */
    return request;
}

unsigned int SQLrequest::get_nbrows(void) const
{
    /** renvoie le nombre d'enregistrements de la requête */
    return nbrows;
}

unsigned int SQLrequest::get_nbfields(void) const
{
    /** renvoie le nombre d'attributs de la requête */
    return nbfields;
}

unsigned int SQLrequest::get_affected(void) const
{
    /** renvoie le nombre de colonnes updated, deleted, or inserted par la dernière requête */
    return affected;
}

unsigned int SQLrequest::get_insert_id(void)
{
    /** Returns the value generated for an AUTO_INCREMENT column by the previous INSERT or UPDATE statement */
    return isValid() ? mysql_insert_id(&mysql) : 0 ;
}

unsigned int SQLrequest::get_field_size(const string& table, const string& field)
{
    /**
    détermine la taille d'un champ d'une table

    @param  table -- nom de la table
    @param  field -- nom du champ
    @return taille du champ 'field'
    */
    unsigned int field_size = 0;

    if ( isValid() ) {
        stringstream request;
        request << "SELECT `CHARACTER_MAXIMUM_LENGTH` ";
        request << "FROM `information_schema`.`COLUMNS` ";
        request << "WHERE COLUMN_NAME IN('" << field << "') ";
        request << "AND TABLE_NAME = '" << table << "';";

        if ( query(request.str()) )
            if ( get_nbrows() )
                field_size = atoi(get_rows().at(0).at(0).data());
        }

    return field_size;
}

vector<vector<string>> SQLrequest::get_rows(void) const
{
    /** renvoie le résultat de la requête sous la forme d'un tableau de caractères
    enregistrements x attributs */
    return rows;
}

bool SQLrequest::query(const string& request)
{
    /**
    exécution d'une requête de type select (met à jour l'attribut rows)

    @retval true si réquête réussie
    @retval false si échec de la requête
    */
    if ( !isValid() || request.size() == 0 )
        return false;

    this->request = request;
    reset_state();

    if ( mysql_query(&mysql, request.data()) )
    {
        last_error << "Request error: " << mysql_error(&mysql) << endl;;
        return false;
    }

    MYSQL_RES*  result = mysql_store_result(&mysql);
    if ( result == nullptr )
    {
        if ( mysql_field_count(&mysql) == 0 )
        {
            // query does not return data
            // (it was not a SELECT)
            set_affected( mysql_affected_rows(&mysql) );
            return true;
        }
        else
            return false;
    }

    set_nbrows( mysql_num_rows(result) );
    set_nbfields( mysql_num_fields(result) );

    rows.clear();
    rows.resize(get_nbrows());
    for (unsigned int i = 0; i < get_nbrows(); i++)
    {
        MYSQL_ROW row = mysql_fetch_row(result);
        for (unsigned int j = 0; j < get_nbfields(); j++)
            rows[i].push_back(row[j]);
    }

    mysql_free_result(result);

    return true;
}

bool SQLrequest::query_fetch(const string& filename, const bool select)
{
    /**
    exécution d'un script sql

    @param  filename -- nom du fichier sql
    @param  select -- requête de type select, met à jour rows (défaut : false)

    @retval true si l'exécution du script a réussi
    @retval false si échec de l'exécution
    */
    ifstream fp(filename);
    if ( !fp )
    {
        last_error << "File error: " << filename << endl;;
        return false;
    }

    string line, request = "";
    while ( getline(fp, line) )
        if ( line.find("--") != 0 )   // commentaire
        {
            if ( line.find(";") == line.size() - 1 ) // fin requête
                request += line + "\n";
            else
                request += line + " ";
        }

    return select ? query(request) : execute(request) ;
}

bool SQLrequest::execute(const string& request)
{
    /**
    exécution d'un ensemble de requêtes

    @param  request -- les requêtes sql

    @retval true si réquête réussie
    @retval false si échec de la requête
    */
    if ( !isValid() )
        return false;

    vector<string>  fetch = format_request(request);
    for (auto it : fetch)
        if ( it.size() )
            if ( mysql_query(&mysql, it.data()) )
            {
                last_error << "Request error: " << it.data() << " " << mysql_error(&mysql) << endl;;
                return false;
            }

    return true;
}

bool SQLrequest::create_db(const string& dba)
{
    /** création de la base de données

    @param  dba -- nom de la base de données

    @retval true si création réussie
    @retval false si échec de la création
    */
    stringstream request;
    request << "drop database if exists `" << dba << "`" << endl;;
    request << "create database `" << dba << "`";

    return execute(request.str());
}

bool SQLrequest::select_db(const string& dba)
{
    /** Causes the database specified by db to become the default (current) database

    @param  dba -- nom de la base de données

    @retval true si création réussie
    @retval false si échec de la création
    */
    if ( !isValid() )
        return false;

    if ( mysql_select_db(&mysql, dba.data()) )
    {
        last_error << "Can't connect to database: " << mysql_error(&mysql) << endl;
        return false;
    }
    return true;
}

const char* SQLrequest::info(void)
{
    /** providing information about the most recently executed statement (or NULL) */
    return isValid() ? mysql_info(&mysql) : nullptr ;
}

void SQLrequest::set_nbrows(unsigned int nbrows)
{
    this->nbrows = nbrows;
}

void SQLrequest::set_nbfields(unsigned int nbfields)
{
    this->nbfields = nbfields;
}

void SQLrequest::set_affected(unsigned int affected)
{
    this->affected = affected;
}

void SQLrequest::reset_state(void)
{
    set_nbrows(0);
    set_nbfields(0);
    set_affected(0);
}

bool SQLrequest::isValid(void) const
{
    return valid;
}

vector<string> SQLrequest::format_request(const string& request)
{
    /**
    formate un ensemble de requêtes

    @param  request -- requête à formater
    @return tableau de liste de requêtes
    */
    vector<string> parsed_list;

    string parsed;
    stringstream input(request);

    while ( getline(input, parsed) )
        parsed_list.push_back(parsed);

    return parsed_list;
}
void SQLrequest::trueQuery(const string& request)
{
    /**
    exécution d'une requête de type select (met à jour l'attribut rows)

    @retval true si réquête réussie
    @retval false si échec de la requête
    */
    /*if ( !isValid() || request.size() == 0 )
        return false;

    this->request = request;
    reset_state();

    if ( mysql_query(&mysql, request.data()) )
    {
        last_error << "Request error: " << mysql_error(&mysql) << endl;;
        return false;
    }
    */
    MYSQL_RES*  result = mysql_store_result(&mysql);
    /*
    if ( result == nullptr )
    {
        if ( mysql_field_count(&mysql) == 0 )
        {
            // query does not return data
            // (it was not a SELECT)
            set_affected( mysql_affected_rows(&mysql) );
            return true;
        }
        else
            return false;
    }*/

    set_nbrows( mysql_num_rows(result) );
    set_nbfields( mysql_num_fields(result) );

    rows.clear();
    rows.resize(get_nbrows());
    for (unsigned int i = 0; i < get_nbrows(); i++)
    {
        MYSQL_ROW row = mysql_fetch_row(result);
        for (unsigned int j = 0; j < get_nbfields(); j++)
            rows[i].push_back(row[j]);
    }

    std::cout << result;
    mysql_free_result(result);
}
//---------------------------------------------------------------------------------
#ifndef BUILD
int main(void)
{
    /** ATTENTION, placer les dll :
    - libmysql.dll
    - libssl-1_1-x64.dll
    - libcrypto-1_1-x64.dll
    dans le répertoire de l'exécutable, sinon crash !!
    */
    /*{
        SQLrequest sql("localhost", "root", "", "dba_test");

        assert( sql.connect() );
        //sql.get_last_error() == "" ;
        assert( sql.close() );
    }

    {
        SQLrequest sql("localhost", "root");

        assert( sql.connect() );
        assert( sql.create_db("dba_test") );
        assert( sql.query_fetch("ut_create.sql") );
        assert( sql.get_last_error() == "" );
        assert( sql.query("SELECT * FROM `personnes`") );
        assert( sql.get_last_request() == "SELECT * FROM `personnes`" );
        assert( sql.get_nbrows() == 10 );
        assert( sql.get_nbfields() == 3 );
        assert( sql.query_fetch("ut_read.sql", true) );
        assert( sql.get_rows().at(0).at(1) == "Nathan Barber" );
        assert( sql.get_rows().at(1).at(0) == "2" );
        assert( sql.get_rows().at(9).at(2) == "Route de Montancy 332" );
        assert( sql.query("INSERT INTO `personnes` VALUES('11', 'Batman', 'Arkham City')") );
        assert( sql.get_affected() == 1 );
        assert( sql.get_insert_id() == 11 );
        assert( sql.get_field_size("personnes", "nom") == 20 );
        assert( sql.get_field_size("personne", "nom") == 0 );

        sql.query("SELECT * FROM `personnes`");
        std::cout << sql.get_rows().at(0).at(1) << std::endl; // affiche une donnée
        assert( sql.close() );
    }*/

    {//Test connection à une base distante.
       SQLrequest sql("172.20.233.59", "admin", "Admin2020*", "base_local", 3306);

        assert( sql.connect() );
        assert( sql.get_last_error() == "") ;
        assert( sql.close() );
    }

    {//Test lecture d'une plaque: (nous affiche une plaque dans la base)
        SQLrequest sql("localhost", "root", "", "base_distante");
        assert( sql.connect() );
        assert( sql.get_last_error() == "");
        // Renvoie True car la plaque existe
        assert( sql.query("SELECT id FROM `information` where plaque = 'YY-111-ZZ'") );
        assert( sql.get_nbrows() == 1 );
        std::cout << sql.get_rows().at(0).at(0) << std::endl;
        // Renvoie False car la plaque existe pas
        assert( sql.query("SELECT id FROM `information` where plaque = 'AA-00-BB'") );
        assert( sql.get_nbrows() == 0 );
        assert( sql.close());
    }

    { // Test la lecture d'une plaque avec une variable
        std::string variable = "AA-000-BB";
        SQLrequest sql("localhost", "root", "", "base_distante");
        assert( sql.connect() );
        assert( sql.get_last_error() == "");
        //test pour savoir si la plaque existe avec une variable.
        assert( sql.query("SELECT id FROM `information` where plaque = variable") );
        assert( sql.get_nbrows() == 1 );
        std::cout << sql.get_rows().at(0).at(0) << std::endl;
    }



    return 0;
}
#endif
