#!/usr/bin/fish

set host challs.tfcctf.com
set port 30148

set key (curl -s -X POST http://$host:$port/enroll | jq -r .key)
echo $key
set tasks_encrypted (curl -s $host:$port/$key/tasks | jq -r .encrypted)
echo $tasks_encrypted
curl -s $host:$port/$key/flag
set tasks (curl -s -X POST $host:$port/$key/decrypt --data "$tasks_encrypted")
set ids (echo $tasks | jq -r '.tasks[].id')
for id in $ids
    curl -X POST $host:$port/$key/task/$id --json '{"output": "AAAA"}'
end
set flag (curl -s $host:$port/$key/flag)
echo $flag
