# WEBSERV BONUS 125

Projet **WEBSERV** (Bonus 125) – serveur web développé dans le cadre de l’école 42.  
Ce serveur est conçu pour être performant, gérer des clients simultanés et supporter une API RESTful complète.

## Fonctionnalités principales

- **Boucle `select`** pour la gestion des clients multiples en **mode non bloquant**  
- **API RESTful**  
- **Méthodes HTTP implémentées** :  
  - `GET` : récupération de ressources  
  - `POST` : création de ressources  
  - `PUT` : mise à jour complète de ressources  
  - `PATCH` : mise à jour partielle  
  - `DELETE` : suppression de ressources  
- Gestion des fichiers statiques et CGI  
- Gestion des erreurs et pages personnalisées  
- Redirections automatiques et index par défaut  
- Configuration flexible via fichier `.conf` pour plusieurs serveurs virtuels  

Configuration flexible via fichier `.conf` pour plusieurs serveurs virtuels  

## Installation

1. Cloner le projet :  
```bash
git clone https://github.com/nadiatafat/WEBSERV.git
```
