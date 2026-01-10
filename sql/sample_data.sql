INSERT INTO users (name, email, role, password_hash, loyalty_level) 
VALUES 
    ('Admin User', 'admin@store.com', 'admin', 'admin123', 1),
    ('Manager User', 'manager@store.com', 'manager', 'manager123', 0),
    ('Customer User', 'customer@store.com', 'customer', 'customer123', 0)
ON CONFLICT (email) DO NOTHING;

INSERT INTO products (name, price, stock_quantity)
VALUES 
    ('Ноутбук', 50000.00, 10),
    ('Смартфон', 30000.00, 20),
    ('Наушники', 5000.00, 50),
    ('Клавиатура', 2000.00, 30),
    ('Монитор', 15000.00, 15)
ON CONFLICT DO NOTHING;