CREATE OR REPLACE FUNCTION audit_order_status_change()
RETURNS TRIGGER AS $$
BEGIN
    IF OLD.status != NEW.status THEN
        INSERT INTO order_status_history 
        (order_id, old_status, new_status, changed_at, changed_by)
        VALUES (NEW.order_id, OLD.status, NEW.status, NOW(), NEW.user_id);
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER order_status_audit
AFTER UPDATE ON orders
FOR EACH ROW
EXECUTE FUNCTION audit_order_status_change();