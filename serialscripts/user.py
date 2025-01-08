class User:
    def __init__(self, username, password_hash, role):
        self.username = username
        self.password_hash = password_hash
        self.role = role

    def __str__(self):
        return f"User(username={self.username}, password_hash={self.password_hash}, role={self.role})"

    @classmethod
    def from_csv_line(cls, line):
        parts = line.split(',')
        if len(parts) != 3:
            raise ValueError(f"Invalid line format: {line}")
        username, password_hash, role = parts
        return cls(username, password_hash, role)
