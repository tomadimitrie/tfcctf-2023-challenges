extension String {
    static func random(length: Int) -> Self {
        let letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
        return Self((0..<length).map{ _ in letters.randomElement()! })
    }
}