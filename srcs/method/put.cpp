#include "Methods.hpp"

HTTPResponse put_file(const std::string &full_path, const ConfigFile &config, const HTTPRequest &request,const std::pair<Session *, bool> &session_info)
{
	struct stat st;
	int code;

	if (stat(full_path.c_str(), &st) == 0)
		code = 200;
	else
		code = 201;

	std::ofstream ofile(full_path.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
	if (!ofile.is_open())
		return (http_response(500, full_path, config, request, session_info));

	std::vector<char>::const_iterator it;
	std::vector<char>::const_iterator ite = request.body.end();
	for (it = request.body.begin(); it != ite; it++)
		ofile << *it;

	if (ofile.fail())
		return (http_response(500, full_path, config, request, session_info));

	ofile.close();

	return (http_response(code, full_path, config, request, session_info));
}

HTTPResponse put_request(const ConfigFile &config, const HTTPRequest &request, EMETHODS method,const std::pair<Session *, bool> &session_info)
{
	std::string full_path = getRootByLocation(request.target, config) + trim_location_root(request.target, config);;

	std::string uri;
	uri = valid_uri(full_path);

	struct stat st;
	if (stat((uri).c_str(), &st) != 0)
		return (http_response(404, full_path, config, request, session_info));

	std::string uri_loc;
	uri_loc = find_location_prefixe(request.target, config);

	if (!is_method_allowed(method, uri_loc, config))
		return (http_response(405, full_path, config, request, session_info));

	if (!is_upload_enabled(uri_loc, config))
		return (http_response(403, full_path, config, request, session_info));

	if (uri.empty())
		return (http_response(409, full_path, config, request, session_info));

	if (get_content_header_code(request))
		return (http_response(get_content_header_code(request), full_path, config, request, session_info));

	if (request.exceedMaxBodySize)
		return (http_response(413, full_path, config, request, session_info));

	if (access(uri.c_str(), W_OK) != 0)
	{
		if (errno == ENOENT)
			return (http_response(404, full_path, config, request, session_info));
		return (http_response(403, full_path, config, request, session_info));
	}

	return (put_file(full_path, config, request, session_info));
}



// | Cas                              | Description                               | Code HTTP                    | Commentaire                                       |
// | -------------------------------- | ----------------------------------------- | ---------------------------- | ------------------------------------------------- |
// | ❌ Méthode non autorisée          | `PUT` pas dans `allow_methods`            | `405 Method Not Allowed`     | Doit aussi renvoyer `Allow: ...`                  |
// | 🔐 Permission refusée            | Pas le droit d’écrire le fichier          | `403 Forbidden`              | Ex: dossier non accessible en écriture            |
// | 📁 URI = dossier                 | URI se termine par `/` ou vise un dossier | `409 Conflict`               | On ne peut pas écrire un fichier nommé `/Photos/` |
// | 🧱 Ressource non modifiable      | Fichier en lecture seule                  | `403 Forbidden`              |                                                   |
// | ❌ `Content-Length` manquant      | Requête sans taille précisée              | `411 Length Required`        | HTTP exige une taille pour `PUT`                  |
// | ❌ `Content-Length` invalide      | Non numérique ou négatif                  | `400 Bad Request`            |                                                   |
// | ⛔ Taille trop grande             | Dépasse `client_max_body_size`            | `413 Payload Too Large`      |                                                   |
// | 💣 Erreur interne d'écriture     | Erreur système pendant `write()`          | `500 Internal Server Error`  |                                                   |
// | 🔀 Mauvais type MIME (optionnel) | Si tu restreins les MIME                  | `415 Unsupported Media Type` |                                                   |

// | Étape | Vérification                                 | Code à retourner |
// | ----- | -------------------------------------------- | ---------------- |
// | 1️⃣   | **Méthode autorisée ?**                      | `405`            |
// | 2️⃣   | **URI valide ? pas un dossier ?**            | `409`            |
// | 3️⃣   | **Header `Content-Length` présent ?**        | `411`            |
// | 4️⃣   | **Header `Content-Length` valide ?**         | `400`            |
// | 5️⃣   | **Taille respecte `client_max_body_size` ?** | `413`            |
// | 6️⃣   | **Droits d’écriture (fichier/dossier) ?**    | `403` ou 404            |
// | 7️⃣   | **(Optionnel) Type MIME autorisé ?**         | `415`            |
// | 8️⃣   | **Erreur pendant l’écriture ?**              | `500`            |

// | Étape | Vérification                                          | Code à retourner             |
// | ----- | ----------------------------------------------------- | ---------------------------- |
// | 1️⃣   | **Méthode autorisée pour cette `location` ?**         | `405 Method Not Allowed`     |
// | 2️⃣   | **URI obsolète, ressource déplacée définitivement ?** | `301 Moved Permanently`      |
// | 3️⃣   | **URI valide ? existe ?**                             | `404 Not Found`              |
// | 4️⃣   | **Header `Content-Length` présent ?**                 | `411 Length Required`        |
// | 5️⃣   | **Header `Content-Length` valide ?**                  | `400 Bad Request`            |
// | 6️⃣   | **Taille respecte `client_max_body_size` ?**          | `413 Payload Too Large`      |
// | 7️⃣   | **Droits d’écriture (dossier ou fichier cible) ?**    | `403 Forbidden`              |
// | 8️⃣   | **Type MIME autorisé ?** (si filtrage)                | `415 Unsupported Media Type` |
// | 9️⃣   | **Erreur lors du traitement (écriture, parsing) ?**   | `500 Internal Server Error`  |
// | 🔟    | **Création réussie**                                  | `201 Created`                |
// | 🔟+1  | **Traitement réussi sans création (ex: mise à jour)** | `200 OK` ou `204 No Content` |

// | Cas d’usage                                         | Méthode | Nécessite `upload_enabled` ? | Doit être listée dans `allow_methods` ? | Destination exacte connue ? |
// | --------------------------------------------------- | ------- | ---------------------------- | --------------------------------------- | --------------------------- |
// | Envoi de fichier sans chemin précis                 | `POST`  | ✅ Oui                        | ✅ Oui                                   | ❌ Non                       |
// | Envoi de données vers un CGI                        | `POST`  | ❌ Non (pas un upload)        | ✅ Oui                                   | ❌ Non                       |
// | Création ou remplacement d’un fichier précis        | `PUT`   | ✅ Oui                        | ✅ Oui                                   | ✅ Oui                       |
// | Mise à jour répétée d’un fichier sans effet de bord | `PUT`   | ✅ Oui                        | ✅ Oui                                   | ✅ Oui                       |
// | Ajout d’un commentaire, enregistrement unique       | `POST`  | ✅ Oui ou ❌ selon le cas      | ✅ Oui                                   | ❌ Non                       |
