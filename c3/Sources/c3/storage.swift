import Foundation
import Crypto

class Client {
    class Task {
        let id = UUID().uuidString
        let command: String
        var output: String? = nil

        init(_ command: String) {
            self.command = command
        }
    }

    let key: String
    let tasks: [Client.Task]

    init(key: String) {
        self.key = key
        self.tasks = [.init("whoami"), .init("id"), .init("hostname"), .init("cat /etc/passwd")]
    }
}

actor ClientStorage {
    var clients = [String: Client]()

    subscript(key: String) -> Client? {
        get { 
            clients[key] 
        }
        set {
            clients[key] = newValue
        }
    }

    func new(_ key: String) {
        self[key] = .init(key: key)
    }

    func get(_ key: String) -> Client? {
        self[key]
    }
}

let storage = ClientStorage()
