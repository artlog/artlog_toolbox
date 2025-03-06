

avoir une définition déclarative des options sous la forme d'un ficher au format json.

une moulinette permet de générer du code C correspondant à la gestion de ce json qui peut être incluse dans un fichier main ou qui génère le fichier main.

Avec l'utilisation de la librarie al_options

définition du binding des variables :


nom:valeur => type varname value

conversion et vérification du type

"nom":{"v1": , "v2", "v3" } => structure ?

ne revient t'on pas à nouveau au problème initial de génération de code de sérialisation ?


pas vraiment, ce n'est pas récursif donc les listes ne contiennent pas de listes.



* Description des arguments, internationalisation.

* Convertisseur d'arguments : permet de convertir des options d'une version ancienne vers une version plus récente ( et réciproquement ? ).


nom type arité varname description

{
"samples":["monfichier 1 2 2"],
"arguments":
[
{
"name":"outputfilename",
"short":"o",
"description":"nom de fichier de sortie",
"type":"string",
"arity":[1],
"default":"-",
"semantic":"out.filename",
"varname":"output"
"checkmethod":"check_output"      ===> une méthode externe check_output est fournie pour vérifier
"castmethod":"cast_output"   ==> une méthode est fournie pour convertir ( toujours depuis char * vers le type final )
},
"--" // EXACT MATCH
,
]
}


[0,1] => optionnel
[1] => obligatoire
[1,3] => liste de 1 à 3 arguments
[3] => trois arguments

Il y a TOUJOURS une limite



arity [0,1] [1] :

char * output

autre :


alerrorcode monprog_parse_options()
{

}

char * monprog_get_output(aloptions * options, int i, char * default)
{

}

int monprog_get_output(aloptions * options, int i, int default)
{
}

main ()
{

	if ( monprog_parse_options(options,args,argv) == AL_EC_OK )
	{
	}
	else
	{
		exit();
	}


}


Il manque des déclarations sur les liens entre les divers arguments, il se peut que nous désirions un outputfile pour chaque infputfile par exemple...


https://www.bortzmeyer.org/8610.html
