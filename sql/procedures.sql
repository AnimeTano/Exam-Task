CREATE OR REPLACE PROCEDURE create_order_procedure(
    p_user_id INTEGER,
    p_product_id INTEGER,
    p_quantity INTEGER
)
LANGUAGE plpgsql
AS $$
DECLARE
    v_order_id INTEGER;
    v_product_price DECIMAL(10,2);
    v_stock_quantity INTEGER;
    v_total_price DECIMAL(10,2);
BEGIN
    BEGIN
        SELECT price, stock_quantity 
        INTO v_product_price, v_stock_quantity
        FROM products 
        WHERE product_id = p_product_id;
        
        IF NOT FOUND THEN
            RAISE EXCEPTION 'Product with ID % not found', p_product_id;
        END IF;
        
        IF v_stock_quantity < p_quantity THEN
            RAISE EXCEPTION 'Insufficient stock. Available: %, Requested: %', 
                            v_stock_quantity, p_quantity;
        END IF;
        
        INSERT INTO orders (user_id, status, total_price)
        VALUES (p_user_id, 'pending', 0)
        RETURNING order_id INTO v_order_id;
        
        INSERT INTO order_items (order_id, product_id, quantity, price)
        VALUES (v_order_id, p_product_id, p_quantity, v_product_price);
        
        v_total_price := p_quantity * v_product_price;
        
        UPDATE orders 
        SET total_price = v_total_price
        WHERE order_id = v_order_id;
        
        UPDATE products 
        SET stock_quantity = stock_quantity - p_quantity
        WHERE product_id = p_product_id;
        
        COMMIT;
        
    EXCEPTION
        WHEN OTHERS THEN
            ROLLBACK;
            RAISE;
    END;
END;
$$;

CREATE OR REPLACE PROCEDURE update_order_status_procedure(
    p_order_id INTEGER,
    p_new_status VARCHAR,
    p_changed_by INTEGER
)
LANGUAGE plpgsql
AS $$
BEGIN
    UPDATE orders 
    SET status = p_new_status
    WHERE order_id = p_order_id;
    
    COMMIT;
EXCEPTION
    WHEN OTHERS THEN
        ROLLBACK;
        RAISE;
END;
$$;