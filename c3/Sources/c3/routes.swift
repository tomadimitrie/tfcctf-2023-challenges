import Vapor
import Crypto

struct EnrollResponse: Content {
    let key: String
}

struct Task: Content {
    let name: String
}

struct TaskResponse: Content {
    let id: String
    let command: String
}

struct TasksResponse: Content {
    let tasks: [TaskResponse]
}

struct FlagResponse: Content {
    let flag: String
}

struct ClientStorageKey: StorageKey {
    typealias Value = Client
}

struct TaskRequest: Content {
    let output: String
}

struct ClientMiddleware: AsyncMiddleware {
    func respond(to request: Request, chainingTo next: AsyncResponder) async throws -> Response {
        let key = request.parameters.get("key")!
        guard let client = await storage.get(key) else {
            throw Abort(.badRequest)
        }
        request.storage[ClientStorageKey.self] = .init(client)
        return try await next.respond(to: request)
    }
}

func routes(_ app: Application) throws {
    app.post("enroll") { req async -> EnrollResponse in
        let key = String.random(length: 16)
        await storage.new(key)
        return .init(
            key: key
        )
    }

    let clients = app.grouped(":key").grouped(ClientMiddleware())

    clients.get("tasks") { req async throws -> EncryptedResponse<TasksResponse> in 
        let client = req.storage[ClientStorageKey.self]!
        let tasks = client.tasks
            .filter { $0.output == nil }
            .map { TaskResponse(id: $0.id, command: $0.command) } 
        guard let response = EncryptedResponse<TasksResponse>(client: client, data: .init(tasks: tasks)) else {
            throw Abort(.internalServerError)
        }
        return response
    }

    clients.post("task", ":id") { req -> HTTPStatus in 
        let client = req.storage[ClientStorageKey.self]!
        let id = req.parameters.get("id")!
        guard let output = try? req.content.decode(TaskRequest.self).output else {
            throw Abort(.badRequest)
        }
        guard let task = client.tasks.first(where: { $0.id == id }) else {
            throw Abort(.badRequest)
        }
        task.output = output
        return .ok
    }

    clients.get("flag") { req -> FlagResponse in
        let client = req.storage[ClientStorageKey.self]!
        guard client.tasks.allSatisfy({ $0.output != nil }) else {
            throw Abort(.unauthorized)
        }
        return .init(flag: ProcessInfo.processInfo.environment["FLAG"]!)
    }

    clients.post("decrypt") { req -> String in
        let client = req.storage[ClientStorageKey.self]!
        let data = Data(base64Encoded: req.body.string!)!
        return String(data: decrypt(client: client, data: data)!, encoding: .utf8)!
    }
}
