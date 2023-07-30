import Foundation
import Crypto
import Vapor

struct EncryptedResponse<T: Content>: Content {
    let encrypted: String
    
    init?(client: Client, data: T) {
        let json = JSONEncoder()
        guard 
            let data = try? json.encode(data),
            let encrypted = encrypt(client: client, data: data) 
        else {
            return nil
        }
        self.encrypted = encrypted.base64EncodedString()
    }
}

func encrypt(client: Client, data: Data) -> Data? {
    guard 
        let keyData = client.key.data(using: .utf8)
    else { return nil}
    let key = SymmetricKey(data: keyData)
    return try? AES.GCM.seal(data, using: key).combined!
}

func decrypt(client: Client, data: Data) -> Data? {
    guard 
        let keyData = client.key.data(using: .utf8)
    else { return nil }
    let key = SymmetricKey(data: keyData)
    return try? AES.GCM.open(.init(combined: data), using: key)
}