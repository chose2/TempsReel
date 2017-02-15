# TempsReel
camera de surveillance

## SetupputeS ##
up l'environnement, aller a https://www.uco.es/investiga/grupos/ava/node/40
télécharger et suiver les instructions

si le makefile trouve pas mmal_core, mmal_util  xécute:  :export LIBRARY_PATH=/opt/vc/lib

##Progresse

simpleppm.ccp enregistre les fichier d'images en .ppm. Gimp est senser etre capable de les ouvrire.
Cette exemple la utilise juste les fonction "still" qui semble pas etre faite pour prendre plusieurs dimage de suite.tiid srueisuStillllitSlp erdnerp ru, c'était surtout pour tester d'autre format et voir combien ca prenait de temps avant de pouvoir voir quelque chose sur la camera (ajustement a la lumiere)

moveDetect est le princiaple fichier de developpement, il contient des tests de fps et permet d'enregistrer une suite d'image.
moveDetect ne se sert pas du module "raspicam_still" il enregistre en .ppm pour les image couleur et .pgm pour les image noir et blanc

http://netpbm.sourceforge.net/doc/ppm.html
et
http://netpbm.sourceforge.net/doc/pgm.html

Ce sont des format simple qui sont senser etre fait et efficace pour l'analyse. (pas efficace en temre d'espace par contre, sa enregistre toute les pixel)

Voici sur quelle type de detection je me lancerait : http://www.hackerfactor.com/blog/index.php?/archives/432-Looks-Like-It.html
ça utilise le noir et blanc et semble simple.

##Ressources
Librairy utilisé : https://github.com/cedricve/raspicam

Format d'image: http://netpbm.sourceforge.net/doc/ppm.html et http://netpbm.sourceforge.net/doc/pgm.html

Detection d'image :
http://www.hackerfactor.com/blog/index.php?/archives/432-Looks-Like-It.html

http://stackoverflow.com/questions/10487152/comparing-two-images-for-motion-detecting-purposes

https://courses.cs.washington.edu/courses/cse576/book/ch9.pdf
