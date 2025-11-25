#!/bin/bash

SERVER_EXEC=./webserv  # Adjust if the binary name is different
CONFIG_PATH=configs/ConfigBase.conf
PORT=4243
LOG_FILE=test_results.log
TMP_FILE=response.tmp
SERVER_PID=

function start_server() {
	echo "[*] Starting webserv with $CONFIG_PATH..."
	$SERVER_EXEC $CONFIG_PATH > /dev/null 2>&1 &
	SERVER_PID=$!
	sleep 1
}

function stop_server() {
	echo "[*] Stopping webserv..."
	kill $SERVER_PID 2>/dev/null
	wait $SERVER_PID 2>/dev/null
}

function test_case() {
	local name="$1"
	local method="$2"
	local path="$3"
	local expected="$4"
	local data="$5"
	local flags="$6"

	echo -n "[$name] "

	if [[ "$method" == "POST" && "$flags" == *"-F"* ]]; then
		# POST multipart/form-data (file upload)
		curl -s -o $TMP_FILE -w "%{http_code}" -X POST $flags http://localhost:$PORT$path > status.txt
	else
		case "$method" in
			GET)
				curl -s -o $TMP_FILE -w "%{http_code}" http://localhost:$PORT$path $flags > status.txt
				;;
			POST)
				curl -s -o $TMP_FILE -w "%{http_code}" -X POST -d "$data" http://localhost:$PORT$path $flags > status.txt
				;;
			DELETE)
				curl -s -o $TMP_FILE -w "%{http_code}" -X DELETE http://localhost:$PORT$path $flags > status.txt
				;;
			PUT)
				curl -s -o $TMP_FILE -w "%{http_code}" -X PUT -T "$data" http://localhost:$PORT$path $flags > status.txt
				;;
			*)
				echo "❌ Invalid method: $method"
				return
				;;
		esac
	fi

	status=$(cat status.txt)

	if [ "$status" = "$expected" ]; then
		echo "✅ Passed (HTTP $status)"
		echo "[$name] Passed (HTTP $status)" >> $LOG_FILE
	else
		echo "❌ Failed (Expected $expected, Got $status)"
		echo "[$name] Failed (Expected $expected, Got $status)" >> $LOG_FILE
	fi
}


# -----------------------------
# Main Test Suite
# -----------------------------
echo "=== Webserv Test Suite ===" > $LOG_FILE
start_server

# Tests
test_case "GET / (index)" GET "/" 200
test_case "GET /Return (redirect)" GET "/Return" 302
test_case "POST /Return (redirect)" POST "/Return" 302 "query=redirect"
test_case "GET /Uploads/" GET "/Uploads/" 200
echo "This is a test file." > upload.txt
test_case "POST /Uploads (multipart file upload)" POST "/Uploads/" 303 "upload.txt" "-F file=@upload.txt"


# Create a file for PUT upload test
echo "File content via PUT" > put_test.txt
test_case "PUT /Uploads/put_file.txt" PUT "/Uploads/put_file.txt" 201 "put_test.txt" "-H Content-Type:text/plain"

# Delete the uploaded file
test_case "DELETE /Uploads/put_file.txt" DELETE "/Uploads/put_file.txt" 204

# Test CGI if scripts exist (adjust if needed)
test_case "GET /cgi-bin/test.sh (CGI)" GET "/cgi-bin/test.sh" 404

test_case "GET /cgi-bin/get_username.py (CGI)" GET "/cgi-bin/get_username.py" 200

# POST to /logout
test_case "POST /logout" POST "/logout" 302 "confirm=true"

# Invalid path
test_case "GET /not_found.html" GET "/not_found.html" 404

stop_server
rm -f $TMP_FILE status.txt put_test.txt

echo "=== Done. See $LOG_FILE for results ==="
