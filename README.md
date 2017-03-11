# TempsReel
camera de surveillance

## Setup
aller a https://www.uco.es/investiga/grupos/ava/node/40
Télécharger et suiver les instructions

si le makefile trouve pas mmal_core, mmal_util  exécute:  :export LIBRARY_PATH=/opt/vc/lib

##Progress

simpleppm.ccp enregistre les fichiers d'images en .ppm. Gimp est sensé etre capable de les ouvrirent.
Cette exemple la utilise juste les fonctions "still" qui semble pas etre faite pour prendre plusieurs images de suite.
C'était surtout pour tester d'autre format et voir combien ça prenait de temps avant de pouvoir voir quelque chose sur la camera (ajustement à la lumière)

moveDetect.cpp est le princiaple fichier de developpement, il contient des tests de fps et permet d'enregistrer une suite d'image.
moveDetect ne se sert pas du module "raspicam_still" il enregistre en .ppm pour les image couleur et .pgm pour les image noir et blanc

En ce moment le cap de vitesse semble être 29-30fps qui est surement sufisant.
http://netpbm.sourceforge.net/doc/ppm.html
et
http://netpbm.sourceforge.net/doc/pgm.html

Ce sont des format simple qui sont senser être fait simple et efficace pour l'analyse. (pas efficace en terme d'espace par contre, ça enregistre toute les pixel)

Voici sur quelle type de detection je pensais me lancer : http://www.hackerfactor.com/blog/index.php?/archives/432-Looks-Like-It.html
ça utilise le noir et blanc et semble simple.

##Ressources
Librairy utilisé : https://github.com/cedricve/raspicam

Format d'image: http://netpbm.sourceforge.net/doc/ppm.html et http://netpbm.sourceforge.net/doc/pgm.html

Detection d'image :
http://codeding.com/articles/motion-detection-algorithm (this one is implemented)

http://www.hackerfactor.com/blog/index.php?/archives/432-Looks-Like-It.html

http://stackoverflow.com/questions/10487152/comparing-two-images-for-motion-detecting-purposes

https://courses.cs.washington.edu/courses/cse576/book/ch9.pdf
