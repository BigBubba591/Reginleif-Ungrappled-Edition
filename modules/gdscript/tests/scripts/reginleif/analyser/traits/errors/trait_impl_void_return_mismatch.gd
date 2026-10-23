trait VoidReturnMismatch

func void_mismatch() -> void

impl for int {
	func void_mismatch() -> int { return self }
}