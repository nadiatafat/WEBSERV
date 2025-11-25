#include "Delete.hpp"
#include <unistd.h>     // access()
#include <cstdio>       // remove()
#include <cerrno>       // errno
#include <cstring>      // strerror()
#include <sys/stat.h>   // stat()
#include <dirent.h>     // directory deletion
#include <cstdlib>
#include "HTTPResponse.hpp"
#include "Methods.hpp"

void tryDeleteProfilePicture(const std::string& path, const std::string& fileName)
{
	std::vector<std::string> valid_extensions;
	valid_extensions.push_back("jpg");
	valid_extensions.push_back("jpeg");
	valid_extensions.push_back("png");
	valid_extensions.push_back("gif");
	valid_extensions.push_back("webp");

	DIR *dir = opendir(path.c_str());
	if (!dir)
	{
		std::cout << RED
			<< "Failed to open directory: " << path
			<< RESET << std::endl;
		return;
	}

	std::string prefix = fileName + ".";

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string filename = entry->d_name;

		if (filename.compare(0, prefix.size(), prefix) == 0)
		{
			std::string full_path = path + "/" + filename;
			if (std::remove(full_path.c_str()) == 0) {
				std::cout << GREEN
					<< "Removed: " << full_path
					<< RESET << std::endl;
			} else {
				std::cout << RED
					<< "Failed to remove: " << full_path
					<< RESET << std::endl;
			}
		}
	}

	closedir(dir);
}

bool deleteDirectory(const std::string& path) //delete directory recusrsive
{
		DIR* dir = opendir(path.c_str()); //ptr to directory stream et open it (opendir returns a DIR* and is used by readdir to read from)
		if (!dir) //if not exist
			return false;

		struct dirent* entry; //declare a ptr to a dirent struct representing an 1 entry at a time (a placeholder to iterate the direcotry 1 item at a time) inside a directory like a subdirectory, a file etc ==here we'll use the dir ptr
		while ((entry = readdir(dir))) //readdir reads 1 file/item at a time from the directory pointed by dir ptr we opended and save it by returning a dirent* structure that contains the name and type of this item we read, if null directory is fully read
		{
			std::string name = entry->d_name; //save the name of entry

			if (name == "." || name == "..") //exclude current and parent directory
				continue;

			std::string fullPath = path + "/" + name;	//build full path of entry

			struct stat st; //to save info about the file
			if (stat(fullPath.c_str(), &st) == 0)
			{
					if (S_ISDIR(st.st_mode))
						deleteDirectory(fullPath); //recursion if its a directory
					else
						remove(fullPath.c_str()); //delete directly if file
			}
		}
		closedir(dir);
		return rmdir(path.c_str()) == 0; //delete the now empty folder itself
}


// std::string resolvePath(const std::string& uri)
// {
// 	const std::string SERVER_ROOT = "./www";
//     if (uri.empty() || uri[0] != '/')
//         return SERVER_ROOT; //default

//     std::string resolved = SERVER_ROOT + uri;

//     size_t i;
//     while ((i = resolved.find("..")) != std::string::npos)
//         resolved.erase(i, 2); // erase ".." from path to keep inside root only

//     return resolved;
// }

// std::string resolvePath(const std::string& uri)
// {
//     std::string raw = "./www" + uri;
//     char real[PATH_MAX];

//     if (realpath(raw.c_str(), real) == NULL) //find true path of raw path on disk and save it to real
//         return ""; // path not found or invalid

//     std::string resolved(real);

//     // check if resolved path is still inside ./www
//     if (resolved.find(realpath("./www", NULL)) != 0)
//         return ""; // protect

//     return resolved;
// }

std::string resolvePath(const std::string& uri)
{
	std::string raw = "./www" + uri;
	char real[PATH_MAX];
	char www_root[PATH_MAX];

	// resolve full path of target
	if (realpath(raw.c_str(), real) == NULL)
		return ""; // not found path

	// Resolve root path
	if (realpath("./www", www_root) == NULL)
		return "";

	std::string resolved(real);
	std::string root(www_root);

	if (resolved.find(root) != 0)
		return ""; // esc root

	return resolved;
}

// void handleDeleteRequest(const HTTPRequest& request, HTTPResponse& response)
// {
// 	std::string uri = request.target;                // /admin
// 	std::string physicalPath = resolvePath(uri);     // from /images/logo.png    TO     ./www/imagines/logo.png

// 	// File or directory oeo ??? and if folder it should have a '/' at end
// 	if (uriNeedsRedirect(uri, physicalPath))
// 	{
// 		response.statusCode = 301;
// 		response.statusMessage = "Moved Permanently";
// 		response.headerFields["Location"] = uri + "/";
// 		return;
// 	}

// 	// permissions
// 	if (access(physicalPath.c_str(), W_OK) != 0)
// 	{
// 		response.statusCode = 403;
// 		response.statusMessage = "Forbidden";
// 		return;
// 	}

// 	// try to delete
// 	bool success = false;
// 	if (isFile(physicalPath))
// 	{
// 		success = (remove(physicalPath.c_str()) == 0);
// 	}
// 	else if (isDirectory(physicalPath))
// 	{
// 		success = deleteDirectory(physicalPath);
// 	}

// 	// check if deleted
// 	if (!success)
// 	{
// 		response.statusCode = 500;
// 		response.statusMessage = "Internal Server Error";
// 		return;
// 	}

// 	// success — 204 No Content
// 	response.statusCode = 204;
// 	response.statusMessage = "No Content";
// 	response.body.clear();
// }

HTTPResponse delete_request(const ConfigFile& config, const HTTPRequest& request, EMETHODS method,const std::pair<Session *, bool> &session_info)
{
	std::string full_path = getRootByLocation(request.target, config) + request.target;
	std::string uri_loc = find_location_prefixe(request.target, config); //find at which location the target is

	struct stat st;
	if (stat(full_path.c_str(), &st) != 0)
		return (http_response(404, full_path, config, request, session_info));

	if (!is_method_allowed(method, uri_loc, config)) //check if delete is allowed in config
		return http_response(405, full_path, config, request, session_info);

	// char resolved_path[PATH_MAX];
	// if (!realpath(full_path.c_str(), resolved_path))
	//     return http_response(404, full_path, config, request, session_info); // path doesn't exist

	// std::string uri(resolved_path);

	//or

	// std::string uri = resolvePath(request.target);
	// if (uri.empty())
   	//     return http_response(404, full_path, config, request, session_info);

	// security check
	// if (uri.find(config.root) != 0)
	//     return http_response(403, full_path, config, request, session_info); // esc root dir

	// if (uriNeedsRedirect(request.target, uri))
	//     return http_response(301, full_path, config, request, session_info);

	// if (access(uri.c_str(), W_OK) != 0)
	//     return http_response(403, full_path, config, request, session_info);

	// if (uriNeedsRedirect(uri, request.target))
	// 	return http_response(301, full_path, config, request, session_info);

	struct stat pathStat; //struct holds metadata about the file-path
	if (stat(full_path.c_str(), &pathStat) == 0) //retrieve data about path and writes it in pathstruct
	{
		if (S_ISREG(pathStat.st_mode))
		{
			if (access(full_path.c_str(), W_OK) != 0)
				return http_response(403, full_path, config, request, session_info);
		}
		else if (S_ISDIR(pathStat.st_mode))
		{
			return http_response(409, full_path, config, request, session_info);
		}
	}
	// else
	// 	return http_response(404, full_path, config, request, session_info);

	if (remove(full_path.c_str()) != 0)
		return http_response(500, full_path, config, request, session_info);

	return http_response(204, full_path, config, request, session_info);
}



// | Étape | Vérification                                          | Code à retourner             |
// | ----- | ----------------------------------------------------- | ---------------------------- |
// | 1️⃣   | **Méthode autorisée pour cette `location` ?**         | `405 Method Not Allowed`     |
// | 2️⃣   | **URI obsolète, ressource déplacée définitivement ?** | `301 Moved Permanently`      |
// | 3️⃣   | **URI valide ? correspond à un fichier existant ?**   | `404 Not Found`              |
// | 4️⃣   | **Tentative de suppression d’un dossier ?**           | `409 Conflict` *(optionnel)* |
// | 5️⃣   | **Fichier accessible (droits système) ?**             | `403 Forbidden`              |
// | 6️⃣   | **Fichier déjà supprimé entre temps ?**               | `404 Not Found`              |
// | 7️⃣   | **Erreur interne lors de la suppression ?**           | `500 Internal Server Error`  |
// | 8️⃣   | **Suppression réussie sans contenu à renvoyer ?**     | `204 No Content`             |
// | 9️⃣   | **Suppression réussie avec message/confirmation ?**   | `200 OK`                     |









