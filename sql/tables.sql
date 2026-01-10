CREATE TABLE IF NOT EXISTS users(
    user_id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    email VARCHAR(70) NOT NULL UNIQUE,
    role VARCHAR(25) CHECK (role IN ('admin', 'manager', 'customer')) NOT NULL,
    password_hash VARCHAR(256) NOT NULL,
    loyalty_level INT DEFAULT 0 CHECK (loyalty_level IN (0, 1))
);

CREATE TABLE IF NOT EXISTS products(
    product_id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    price DECIMAL(10,2) CHECK (price > 0),
    stock_quantity INT CHECK (stock_quantity >= 0)
);

CREATE TABLE IF NOT EXISTS orders(
    order_id SERIAL PRIMARY KEY,
    user_id INT REFERENCES users(user_id),
    status VARCHAR(50) CHECK (status IN ('pending', 'completed', 'canceled', 'returned')),
    total_price DECIMAL(10, 2) default 0,
    order_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS order_items(
    order_item_id SERIAL PRIMARY KEY,
    order_id INT REFERENCES orders(order_id),
    product_id INT REFERENCES products(product_id),
    quantity INT CHECK (quantity > 0) NOT NULL,
    price DECIMAL(10, 2) CHECK (price >= 0)
);

CREATE TABLE IF NOT EXISTS order_status_history(
    history_id SERIAL PRIMARY KEY,
    order_id INT REFERENCES orders(order_id),
    old_status VARCHAR(100),
    new_status VARCHAR(100),
    changed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    changed_by INT REFERENCES users(user_id)
);

CREATE TABLE IF NOT EXISTS audit_log (
    log_id SERIAL PRIMARY KEY,
    entity_type VARCHAR(20) CHECK (entity_type IN ('order', 'product', 'user', 'system')) NOT NULL,
    entity_id INT,
    operation VARCHAR(20) CHECK (operation in ('insert', 'update', 'delete', 'startup', 'login', 'logout')) NOT NULL,
    performed_by INT REFERENCES users(user_id),
    performed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    details TEXT
);