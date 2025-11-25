// #include "Methods.hpp"

// std::string extension(const std::string &type)
// {
// 	std::string extension;

// 	std::string types[] =
// 	{
// 		"text/html; charset=UTF-8",
// 		"text/html; charset=UTF-8",
// 		"text/css; charset=UTF-8",
// 		"application/javascript",
// 		"application/json ; charset=UTF-8",
// 		"image/jpeg",
// 		"image/jpeg",
// 		"image/png",
// 		"application/pdf"
// 	};

// 	std::string extensions[] =
// 	{
// 		"html", "htm",
// 		"css",
// 		"js",
// 		"json",
// 		"jpg", "jpeg", "png",
// 		"pdf"
// 	};


// 	size_t count = sizeof(extensions) / sizeof(extensions[0]);

// 	for (size_t i = 0; i < count; i++)
// 	{
// 		if (type == types[i])
// 			return ("." + extensions[i]);
// 	}

// 	return ".txt";
// }

// static std::string create_name(const std::string &full_path, const HTTPRequest &request)
// {
// 	DIR *dir = opendir(full_path.c_str());
// 	struct dirent *input;
// 	int count = 0;
// 	std::stringstream stream;

// 	while ((input = readdir(dir)))
// 	{
// 		std::string file_name;
// 		file_name = input->d_name;
// 		if (file_name == "." || file_name == "..")
// 			continue;
// 		count++;
// 	}
// 	stream << count;
// 	closedir(dir);
// 	return (full_path + "/" + stream.str() + extension(request.headerFields.at("content-type")));
// }

// HTTPResponse post_file(const std::string &full_path, const ConfigFile &config, const HTTPRequest &request)
// {
// 	struct stat st;
// 	int code;

// 	if (stat(full_path.c_str(), &st) == 0)
// 		code = 200;
// 	else
// 		code = 201;

// 	std::string file_name = create_name(full_path, request, session_info);
// 	std::ofstream ofile(file_name.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
// 	if (!ofile.is_open())
// 		return (http_response(500, full_path, config, request, session_info));

// 	std::cout << "coucou la mif" << create_name(full_path, request, session_info) << std::endl;

// 	std::vector<char>::const_iterator it;
// 	std::vector<char>::const_iterator ite = request.body.end();
// 	for (it = request.body.begin(); it != ite; it++)
// 		ofile << *it;

// 	if (ofile.fail())
// 		return (http_response(500, full_path, config, request, session_info));

// 	ofile.close();

// 	return (http_response(code, file_name, config, request, session_info));
// }


// HTTPResponse patch_request(const ConfigFile &config, const HTTPRequest &request, EMETHODS method)
// {
// 	std::string full_path = config.root + request.target;

// 	std::string uri_loc;
// 	uri_loc = find_location_prefixe(request.target, config);

// 	if (!is_method_allowed(method, uri_loc, config))
// 		return (http_response(405, full_path, config, request, session_info));

// 	if (!is_upload_enabled(uri_loc, config))
// 		return (http_response(403, full_path, config, request, session_info));

// 	struct stat st;

// 	if (stat(full_path.c_str(), &st) != 0)
// 		return (http_response(404, full_path, config, request, session_info));

// 	if (get_content_header_code(request))
// 		return (http_response(get_content_header_code(request), full_path, config, request, session_info));

// 	if (request.exceedMaxBodySize)
// 		return (http_response(413, full_path, config, request, session_info));

// 	if (access(full_path.c_str(), W_OK) != 0)
// 		return (http_response(403, full_path, config, request, session_info));

// 	//AJOUTER MIME + TARD

// 	return (post_file(full_path, config, request, session_info));
// }

// // | Cas                              | Description                               | Code HTTP                    | Commentaire                                       |
// // | -------------------------------- | ----------------------------------------- | ---------------------------- | ------------------------------------------------- |
// // | ❌ Méthode non autorisée          | `PUT` pas dans `allow_methods`            | `405 Method Not Allowed`     | Doit aussi renvoyer `Allow: ...`                  |
// // | 🔐 Permission refusée            | Pas le droit d’écrire le fichier          | `403 Forbidden`              | Ex: dossier non accessible en écriture            |
// // | 📁 URI = dossier                 | URI se termine par `/` ou vise un dossier | `409 Conflict`               | On ne peut pas écrire un fichier nommé `/Photos/` |
// // | 🧱 Ressource non modifiable      | Fichier en lecture seule                  | `403 Forbidden`              |                                                   |
// // | ❌ `Content-Length` manquant      | Requête sans taille précisée              | `411 Length Required`        | HTTP exige une taille pour `PUT`                  |
// // | ❌ `Content-Length` invalide      | Non numérique ou négatif                  | `400 Bad Request`            |                                                   |
// // | ⛔ Taille trop grande             | Dépasse `client_max_body_size`            | `413 Payload Too Large`      |                                                   |
// // | 💣 Erreur interne d'écriture     | Erreur système pendant `write()`          | `500 Internal Server Error`  |                                                   |
// // | 🔀 Mauvais type MIME (optionnel) | Si tu restreins les MIME                  | `415 Unsupported Media Type` |                                                   |

// // | Étape | Vérification                                 | Code à retourner |
// // | ----- | -------------------------------------------- | ---------------- |
// // | 1️⃣   | **Méthode autorisée ?**                      | `405`            |
// // | 2️⃣   | **URI valide ? pas un dossier ?**            | `409`            |
// // | 3️⃣   | **Header `Content-Length` présent ?**        | `411`            |
// // | 4️⃣   | **Header `Content-Length` valide ?**         | `400`            |
// // | 5️⃣   | **Taille respecte `client_max_body_size` ?** | `413`            |
// // | 6️⃣   | **Droits d’écriture (fichier/dossier) ?**    | `403` ou 404            |
// // | 7️⃣   | **(Optionnel) Type MIME autorisé ?**         | `415`            |
// // | 8️⃣   | **Erreur pendant l’écriture ?**              | `500`            |


// // | Cas d’usage                                         | Méthode | Nécessite `upload_enabled` ? | Doit être listée dans `allow_methods` ? | Destination exacte connue ? |
// // | --------------------------------------------------- | ------- | ---------------------------- | --------------------------------------- | --------------------------- |
// // | Envoi de fichier sans chemin précis                 | `POST`  | ✅ Oui                        | ✅ Oui                                   | ❌ Non                       |
// // | Envoi de données vers un CGI                        | `POST`  | ❌ Non (pas un upload)        | ✅ Oui                                   | ❌ Non                       |
// // | Création ou remplacement d’un fichier précis        | `PUT`   | ✅ Oui                        | ✅ Oui                                   | ✅ Oui                       |
// // | Mise à jour répétée d’un fichier sans effet de bord | `PUT`   | ✅ Oui                        | ✅ Oui                                   | ✅ Oui                       |
// // | Ajout d’un commentaire, enregistrement unique       | `POST`  | ✅ Oui ou ❌ selon le cas      | ✅ Oui                                   | ❌ Non                       |


#include "ConfigFile.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Methods.hpp"


HTTPResponse patch_request(const ConfigFile &config, const HTTPRequest &request, EMETHODS method,const std::pair<Session *, bool> &session_info)
{
	std::string full_path = getRootByLocation(request.target, config) + request.target;

	std::string uri_loc = find_location_prefixe(request.target, config);

	if (!is_method_allowed(method, uri_loc, config))
		return http_response(405, full_path, config, request, session_info);

	struct stat st;
	if (stat(full_path.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
		return http_response(409, full_path, config, request, session_info);

	if (request.headerFields.find("content-length") == request.headerFields.end())
		return http_response(411, full_path, config, request, session_info);

	const std::string &cl = request.headerFields.at("content-length");

	for (size_t i = 0; i < cl.length(); ++i)
	{
		if (!isdigit(cl[i]))
			return http_response(400, full_path, config, request, session_info);
	}

	if (request.exceedMaxBodySize)
		return http_response(413, full_path, config, request, session_info);

	if (stat(full_path.c_str(), &st) != 0)
		return http_response(404, full_path, config, request, session_info);

	if (access(full_path.c_str(), W_OK) != 0)
		return http_response(403, full_path, config, request, session_info);

	// append to patch
	std::ofstream outfile(full_path.c_str(), std::ios::out | std::ios::app | std::ios::binary);
	if (!outfile.is_open())
		return http_response(500, full_path, config, request, session_info);

	outfile.write(request.body.data(), request.body.size());
	if (outfile.fail())
		return http_response(500, full_path, config, request, session_info);
	outfile.close();

	return http_response(200, full_path, config, request, session_info);
}
