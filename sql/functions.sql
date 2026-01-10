CREATE OR REPLACE FUNCTION update_order_date()
RETURNS TRIGGER AS $$
BEGIN
    IF OLD.status IS DISTINCT FROM NEW.status THEN
        NEW.order_date = CURRENT_TIMESTAMP;
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;


DROP TRIGGER IF EXISTS trigger_update_order_date ON orders;
CREATE TRIGGER trigger_update_order_date
BEFORE UPDATE ON orders
FOR EACH ROW
EXECUTE FUNCTION update_order_date();


CREATE OR REPLACE FUNCTION update_order_totals()
RETURNS TRIGGER AS $$
BEGIN
    IF OLD.price IS DISTINCT FROM NEW.price THEN
        UPDATE orders o
        SET total_price = (
            SELECT COALESCE(SUM(oi.quantity * oi.price), 0)
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
$$ LANGUAGE plpgsql;


DROP TRIGGER IF EXISTS trigger_update_order_totals ON products;
CREATE TRIGGER trigger_update_order_totals
AFTER UPDATE ON products
FOR EACH ROW
EXECUTE FUNCTION update_order_totals();


CREATE OR REPLACE FUNCTION update_order_status_history()
RETURNS TRIGGER AS $$
BEGIN
    IF OLD.status IS DISTINCT FROM NEW.status THEN
        INSERT INTO order_status_history 
        (order_id, old_status, new_status, changed_at, changed_by)
        VALUES (NEW.order_id, OLD.status, NEW.status, NOW(), NEW.user_id);
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;


DROP TRIGGER IF EXISTS trigger_order_status_history ON orders;
CREATE TRIGGER trigger_order_status_history
AFTER UPDATE ON orders
FOR EACH ROW
EXECUTE FUNCTION update_order_status_history();


CREATE OR REPLACE FUNCTION audit_products()
RETURNS TRIGGER AS $$
BEGIN
    IF TG_OP = 'INSERT' THEN
        INSERT INTO audit_log (entity_type, entity_id, operation, performed_by, performed_at, details)
        VALUES ('product', NEW.product_id, 'insert', 1, NOW(), 'Product created: ' || NEW.name);
        RETURN NEW;
    ELSIF TG_OP = 'UPDATE' THEN
        INSERT INTO audit_log (entity_type, entity_id, operation, performed_by, performed_at, details)
        VALUES ('product', NEW.product_id, 'update', 1, NOW(), 'Product updated: ' || NEW.name);
        RETURN NEW;
    ELSIF TG_OP = 'DELETE' THEN 
        INSERT INTO audit_log (entity_type, entity_id, operation, performed_by, performed_at, details)
        VALUES ('product', OLD.product_id, 'delete', 1, NOW(), 'Product deleted: ' || OLD.name);
        RETURN OLD;
    END IF;
    RETURN NULL;
END;
$$ LANGUAGE plpgsql;


DROP TRIGGER IF EXISTS trigger_audit_products ON products;
CREATE TRIGGER trigger_audit_products
AFTER INSERT OR UPDATE OR DELETE ON products
FOR EACH ROW
EXECUTE FUNCTION audit_products();


CREATE OR REPLACE FUNCTION audit_orders()
RETURNS TRIGGER AS $$
BEGIN
    IF TG_OP = 'INSERT' THEN
        INSERT INTO audit_log (entity_type, entity_id, operation, performed_by, performed_at, details)
        VALUES ('order', NEW.order_id, 'insert', NEW.user_id, NOW(), 'Order created');
        RETURN NEW;
    ELSIF TG_OP = 'UPDATE' THEN
        INSERT INTO audit_log (entity_type, entity_id, operation, performed_by, performed_at, details)
        VALUES ('order', NEW.order_id, 'update', NEW.user_id, NOW(), 'Order status changed to: ' || NEW.status);
        RETURN NEW;
    ELSIF TG_OP = 'DELETE' THEN
        INSERT INTO audit_log (entity_type, entity_id, operation, performed_by, performed_at, details)
        VALUES ('order', OLD.order_id, 'delete', OLD.user_id, NOW(), 'Order deleted');
        RETURN OLD;
    END IF;
    RETURN NULL;
END;
$$ LANGUAGE plpgsql;


DROP TRIGGER IF EXISTS trigger_audit_orders ON orders;
CREATE TRIGGER trigger_audit_orders
AFTER INSERT OR UPDATE OR DELETE ON orders
FOR EACH ROW
EXECUTE FUNCTION audit_orders();


CREATE OR REPLACE FUNCTION audit_users()
RETURNS TRIGGER AS $$
BEGIN
    IF TG_OP = 'INSERT' THEN
        INSERT INTO audit_log (entity_type, entity_id, operation, performed_by, performed_at, details)
        VALUES ('user', NEW.user_id, 'insert', 1, NOW(), 'User created: ' || NEW.name);
        RETURN NEW;
    ELSIF TG_OP = 'UPDATE' THEN
        INSERT INTO audit_log (entity_type, entity_id, operation, performed_by, performed_at, details)
        VALUES ('user', NEW.user_id, 'update', 1, NOW(), 'User updated: ' || NEW.name);
        RETURN NEW;
    ELSIF TG_OP = 'DELETE' THEN
        INSERT INTO audit_log (entity_type, entity_id, operation, performed_by, performed_at, details)
        VALUES ('user', OLD.user_id, 'delete', 1, NOW(), 'User deleted: ' || OLD.name);
        RETURN OLD;
    END IF;
    RETURN NULL;
END;
$$ LANGUAGE plpgsql;


DROP TRIGGER IF EXISTS trigger_audit_users ON users;
CREATE TRIGGER trigger_audit_users
AFTER INSERT OR UPDATE OR DELETE ON users
FOR EACH ROW
EXECUTE FUNCTION audit_users();