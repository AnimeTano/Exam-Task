CREATE FUNCTION update_order_date()
RETURNS TRIGGER AS $$

BEGIN
    IF OLD.status IS DISTINCT FROM NEW.status THEN
        NEW.order_date = CURRENT_TIMESTAMP;
    END IF;
    RETURN NEW;
END;


CREATE TRIGGER trigger_update_order_date
BEFORE UPDATE ON orders
FOR EACH ROW
EXECUTE FUNCTION update_order_date();



CREATE FUNCTION update_order_totals()
RETURNS TRIGGER AS $$
BEGIN
    IF OLD.price IS DISTINCT FROM NEW.price THEN
        UPDATE orders order_date
        SET total_price = (
            SELECT SUM(oi.quantity * oi.price)
            FROM order_items oi
            WHERE oi.order_id = o.order_id
        )
        WHERE o.order_id IN (
            SELECT order_id FROM order_items
            WHERE product_id = NEW.product_id
        );
    END IF;
    RETURN NEW;
END;

CREATE TRIGGER trigger_update_order_totals
AFTER UPDATE ON products
FOR EACH ROW
EXECUTE FUNCTION update_order_totals();



CREATE FUNCTION audit_products()
RETURNS TRIGGER AS $$
BEGIN
    IF TYPE_OPERATION = 'INSERT' THEN
        INSERT INTO audit_log (entity_type, entity_id, operation, performed_by, details)
        VALUES ('product', NEW.product_id, 'insert', 1, 'Product created: ' || NEW.name);
        RETURN NEW;
    ELSIF TYPE_OPERATION = 'UPDATE' THEN
        INSERT INTO audit_log (entity_type, entity_id, operation, performed_by, details)
        VALUES ('product', NEW.product_id, 'update', 1, 'Product updated: ' || NEW.name);
        RETURN NEW;
    ELSIF TYPE_OPERATION = 'DELETE' THEN 
        INSERT INTO audit_log (entity_type, entity_id, operation, performed_by, details)
        VALUES ('product', OLD.product_id, 'delete', 1, 'Product deleted: ' || OLD.name);
        RETURN OLD;
    END IF;
    RETURN NULL;
END;


CREATE TRIGGER trigger_audit_products
AFTER INSERT OR UPDATE OR DELETE ON audit_products
FOR EACH ROW
EXECUTE FUNCTION audit_products();



CREATE OR REPLACE FUNCTION audit_orders()
RETURNS TRIGGER AS $$

BEGIN
    IF TG_OP = 'INSERT' THEN
        INSERT INTO audit_log (entity_type, entity_id, operation, performed_by, details)
        VALUES ('order', NEW.order_id, 'insert', NEW.user_id, 'Order created');
        RETURN NEW;
    ELSIF TG_OP = 'UPDATE' THEN
        INSERT INTO audit_log (entity_type, entity_id, operation, performed_by, details)
        VALUES ('order', NEW.order_id, 'update', NEW.user_id, 'Order status: ' || NEW.status);
        RETURN NEW;
    ELSIF TG_OP = 'DELETE' THEN
        INSERT INTO audit_log (entity_type, entity_id, operation, performed_by, details)
        VALUES ('order', OLD.order_id, 'delete', OLD.user_id, 'Order deleted');
        RETURN OLD;
    END IF;
    RETURN NULL;
END;

CREATE TRIGGER trigger_audit_orders
AFTER INSERT OR UPDATE OR DELETE ON orders
FOR EACH ROW
EXECUTE FUNCTION audit_orders();



CREATE OR REPLACE FUNCTION audit_users()
RETURNS TRIGGER AS $$
BEGIN
    IF TG_OP = 'INSERT' THEN
        INSERT INTO audit_log (entity_type, entity_id, operation, performed_by, details)
        VALUES ('user', NEW.user_id, 'insert', 1, 'User created: ' || NEW.name);
        RETURN NEW;
    ELSIF TG_OP = 'UPDATE' THEN
        INSERT INTO audit_log (entity_type, entity_id, operation, performed_by, details)
        VALUES ('user', NEW.user_id, 'update', 1, 'User updated: ' || NEW.name);
        RETURN NEW;
    ELSIF TG_OP = 'DELETE' THEN
        INSERT INTO audit_log (entity_type, entity_id, operation, performed_by, details)
        VALUES ('user', OLD.user_id, 'delete', 1, 'User deleted: ' || OLD.name);
        RETURN OLD;
    END IF;
    RETURN NULL;
END;

CREATE TRIGGER trigger_audit_users
AFTER INSERT OR UPDATE OR DELETE ON users
FOR EACH ROW
EXECUTE FUNCTION audit_users();