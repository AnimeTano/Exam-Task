CREATE OR REPLACE FUNCTION getOrderStatus(order_id_param INTEGER)
RETURNS VARCHAR AS $$
DECLARE
    order_status VARCHAR;
BEGIN
    SELECT status INTO order_status 
    FROM orders 
    WHERE order_id = order_id_param;
    
    RETURN COALESCE(order_status, 'not found');
END;
$$ LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION getUserOrderCount(user_id_param INTEGER)
RETURNS INTEGER AS $$
DECLARE
    order_count INTEGER;
BEGIN
    SELECT COUNT(*) INTO order_count 
    FROM orders 
    WHERE user_id = user_id_param;
    
    RETURN COALESCE(order_count, 0);
END;
$$ LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION getTotalSpentByUser(user_id_param INTEGER)
RETURNS DECIMAL AS $$
DECLARE
    total_spent DECIMAL(10,2);
BEGIN
    SELECT COALESCE(SUM(total_price), 0) INTO total_spent
    FROM orders 
    WHERE user_id = user_id_param 
      AND status IN ('completed', 'returned');
    
    RETURN total_spent;
END;
$$ LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION canReturnOrder(order_id_param INTEGER)
RETURNS BOOLEAN AS $$
DECLARE
    v_status VARCHAR;
    v_order_date TIMESTAMP;
    v_days_passed INTEGER;
BEGIN
    SELECT status, order_date INTO v_status, v_order_date
    FROM orders 
    WHERE order_id = order_id_param;
    
    IF NOT FOUND THEN
        RETURN FALSE;
    END IF;
    
    IF v_status != 'completed' THEN
        RETURN FALSE;
    END IF;
    
    v_days_passed := EXTRACT(DAY FROM (NOW() - v_order_date));
    
    IF v_days_passed > 30 THEN
        RETURN FALSE;
    END IF;
    
    RETURN TRUE;
END;
$$ LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION getOrderStatusHistory(order_id_param INTEGER)
RETURNS TABLE(
    history_id INTEGER,
    order_id INTEGER,
    old_status VARCHAR,
    new_status VARCHAR,
    changed_at TIMESTAMP,
    changed_by INTEGER,
    user_name VARCHAR
) AS $$
BEGIN
    RETURN QUERY
    SELECT 
        osh.history_id,
        osh.order_id,
        osh.old_status,
        osh.new_status,
        osh.changed_at,
        osh.changed_by,
        u.name as user_name
    FROM order_status_history osh
    LEFT JOIN users u ON osh.changed_by = u.user_id
    WHERE osh.order_id = order_id_param
    ORDER BY osh.changed_at DESC;
END;
$$ LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION getAuditLogByUser(user_id_param INTEGER)
RETURNS TABLE(
    log_id INTEGER,
    entity_type VARCHAR,
    entity_id INTEGER,
    operation VARCHAR,
    performed_by INTEGER,
    performed_at TIMESTAMP,
    details TEXT,
    performer_name VARCHAR
) AS $$
BEGIN
    RETURN QUERY
    SELECT 
        al.log_id,
        al.entity_type,
        al.entity_id,
        al.operation,
        al.performed_by,
        al.performed_at,
        al.details,
        u.name as performer_name
    FROM audit_log al
    LEFT JOIN users u ON al.performed_by = u.user_id
    WHERE al.performed_by = user_id_param
    ORDER BY al.performed_at DESC;
END;
$$ LANGUAGE plpgsql;