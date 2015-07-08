<?php

function productividad_realtime(){
    $time =REQUEST_TIME - (SIETE_DIAS * 10);
    $date_string=format_date($time,'custom','Y-m-d');
    $valores=get_valores_for_time($time);
    $prod_pera=$valores['volumenes']['pera'];
    $prod_mzn=$valores['volumenes']['mnz'];


    $result=db_query("SELECT
          c.LINEA     AS panta,
          env.ESPECIE  AS  especie,
         COUNT(DISTINCT c.EMBALADOR) AS embaladores,
          SUM(emb.EQ)      AS cajas,
          SUM(emb.EQ * env.EQ) AS cajas_eq,
          IF(MAX(env.ESPECIE) =1, {$prod_mzn},{$prod_pera})* COUNT(DISTINCT c.EMBALADOR) AS prod_esperada,
          (SUM(emb.EQ ) - IF(MAX(env.ESPECIE) =1, {$prod_mzn},{$prod_pera})* COUNT(DISTINCT c.EMBALADOR)) AS diff_cajas,
          (SUM(emb.EQ * env.EQ) - ( IF(MAX(env.ESPECIE) =1, {$prod_mzn},{$prod_pera})* COUNT(DISTINCT c.EMBALADOR))) * AVG(env.EQ) AS diff_eq
           
        FROM (((fruitid.cajas c
             JOIN fruitid.embaladores e
               ON ((e.EMBALADOR = c.EMBALADOR)))
            JOIN fruitid.envases env
              ON ((c.ENVASE = env.ENVASE)))
           JOIN fruitid.embalados emb
             ON ((c.EMBALADO = emb.EMBALADO)))
        WHERE ((c.EMPRESA = 1)
               AND (c.PALLET >= 0))
        AND DATE(INGRESO)='".$date_string."'
         GROUP BY DATE(INGRESO), c.linea");

    foreach ($result as $item){
           if(!@$table['header'])$table['header']=array_keys((array)$item);

          $table['rows'][]=array_values((array)$item);
    }
    $return= theme('table',$table);
    unset($table);

    $result=db_query("SELECT
          c.LINEA     AS planta,
          env.ESPECIE  AS  especie,

        env.nombre AS envase,
        emb.nombre AS embalado,
        
          SUM(emb.EQ)      AS cajas,
          SUM(emb.EQ * env.EQ) AS cajas_eq
           
        FROM (((fruitid.cajas c
             JOIN fruitid.embaladores e
               ON ((e.EMBALADOR = c.EMBALADOR)))
            JOIN fruitid.envases env
              ON ((c.ENVASE = env.ENVASE)))
           JOIN fruitid.embalados emb
             ON ((c.EMBALADO = emb.EMBALADO)))
        WHERE ((c.EMPRESA = 1)
               AND (c.PALLET >= 0))
        AND DATE(INGRESO)='{$date_string}' 
         GROUP BY DATE(INGRESO) , env.ESPECIE,env.ENVASE, emb.EMBALADO
        order by ingreso

         ");

    foreach ($result as $item){
           if(!@$table['header'])$table['header']=array_keys((array)$item);

          $table['rows'][]=array_values((array)$item);
    }
    $return.= theme('table',$table);


    return $return;


}
