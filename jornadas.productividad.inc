<?php

function bulk_process_poductividad($form,&$form_state){
    $from['coment']=array('#markup'=>'Regerar los datos.Borra las cofiguraciones previas');

    $from['start_time']=array('#type'=>'date', '#title'=>'inicio');
    $from['end_time']=array('#type'=>'date', '#title'=>'final');

    $from['enviar']=array('#type'=>'submit', '#value'=>'procesar');
    return $from;
}
function bulk_process_poductividad_validate($form,&$form_state){
  $form_state['values']['start_time']=mktime(0,0,0,$form_state['values']['start_time']['month'],$form_state['values']['start_time']['day'],$form_state['values']['start_time']['year']);
  $form_state['values']['end_time']=mktime(23,59,58,$form_state['values']['end_time']['month'],$form_state['values']['end_time']['day'],$form_state['values']['end_time']['year']);

}
function bulk_process_poductividad_submit($form,&$form_state){
    
    $jornadas = new EntityFieldQuery();

    $jornadas->entityCondition('entity_type', 'data_jornadas')
      ->propertyCondition('start_time',$form_state['values']['start_time'],'>=')
      ->propertyCondition('end_time',$form_state['values']['end_time'],'<=');
    $result = $jornadas->execute();

    if(empty($result['data_jornadas'])){
        drupal_set_message('sin informacion de productividad que procesar');
        return;
    }   
    foreach ($result['data_jornadas'] as $key => $value) {

      $operations[]=array('productividad_import_jid',array($key));
      $operations[]=array('productividad_planta_import_jid',array($key));
      $operations[]=array('set_planta_personal_jornada',array($key));
      $operations[]=array('set_productividad_jornada_otros',array($key));



    }
    
     $batch = array(
    'operations' => $operations,
    'finished' => 'batch_prod_finished',
    
  );

    batch_set($batch);
        batch_process('jornadas-laborales');


}
function batch_prod_finished(){
}


function set_planta_personal_jornada($jid){
    db_query("DELETE FROM  planta_personal_jornada  WHERE jid=:jid ",array(':jid' => $jid));

    $jornada=array_shift(entity_load('data_jornadas',array($jid)));
    $valores=get_valores_for_time($jornada->start_time);
    $day_number=date('N',$timestamp);


    $ideal_time_work=$valores['horasdia'][$day_number] * 0.95;
    db_query("INSERT INTO planta_personal_jornada (jid,pid,time_work,planta,ideal_time_work , work_type,numeric_work)
                SELECT jid,pid , SUM(time_total) AS time_work ,planta, :ideal_time_work *3600 AS ideal_time_work , work_type,
                CASE 
                  WHEN SUM(time_total) /3600  > :ideal_time_work THEN 1 
                  WHEN SUM(time_total) /3600  < :ideal_time_work AND SUM(time_total) /3600  > 4 THEN .5 
                  ELSE 0
                END AS numeric_work

                FROM personal_jornada WHERE jid=:jid
                GROUP BY pid", array(':jid'=>$jid, ':ideal_time_work'=>$ideal_time_work));

  /*  $result=db_query("SELECT pid,work_type,planta , SUM(time_total) AS time_total
                         FROM personal_jornada WHERE jid=:jid
                        GROUP BY pid",array(':jid' => $jid));
    foreach ($result as $j) {

        $planta_personal_jornada=new stdClass();

        $hours_work=$j->time_total/3600;

        if($hours_work>$ideal_time_work){
          $planta_personal_jornada->numeric_work=1; 
        } else if($hours_work<$ideal_time_work && $hours_work>($ideal_time_work /2)){
          $planta_personal_jornada->numeric_work=0.5; 
        } else {          
          $planta_personal_jornada->numeric_work=0; 
        }
        $planta_personal_jornada->pid=$j->pid;
        $planta_personal_jornada->jid=$jornada->jid;
        $planta_personal_jornada->time_work=$j->time_total;
        $planta_personal_jornada->planta=$j->planta;
        $planta_personal_jornada->ideal_time_work=$valores['horasdia'][$day_number] *3600;
        $planta_personal_jornada->work_type=$j->work_type;
        drupal_write_record('planta_personal_jornada',$planta_personal_jornada);


    }*/
}
function set_productividad_jornada_otros($jid){
    $jornada=array_shift(entity_load('data_jornadas',array($jid)));

    $valores=get_valores_for_time($jornada->start_time);

    $productividad_por_planta=db_select('planta_jornada_productividad','pp')->fields('pp',array('planta','monto'))->condition('jid',$jid)->execute()->fetchAllKeyed(0,1);

    $horas_por_planta=db_query("SELECT planta ,SUM(numeric_work) total
                                          FROM planta_personal_jornada 
                                          WHERE jid=:jid
                                          AND work_type IN ('varios','acomodador')
                                          GROUP BY planta
                                          ",array(':jid' => $jid))->fetchAllKeyed(0,1);
    $valor_hora[1]=$productividad_por_planta[1]/$horas_por_planta[1];
    $valor_hora[2]=$productividad_por_planta[2]/$horas_por_planta[2];
 
  /*  $empleados=db_select('planta_personal_jornada','ppj')->fields('ppj')->condition('work_type',array('varios','acomodador'))
                ->condition('jid',$jid)->execute();
     echo "b5 " .(time() - $time) ."<br>";
    $time=time();

    while ( $empleado = $empleados->fetchAssoc()) {
      $monto=$valor_hora[$empleado['planta']]*$empleado['numeric_work'];

      db_insert('personal_jornada_productividad')->
        fields(array(
                "jid"=>$jid,
                "pid"=>$empleado['pid'],
                "tipo"=>'resto',
                "cajas"=>0,
                "cajas_eq"=>0,
                "diff_cajas"=>0,
                "diff_eq"=>0,
                "coeff"=>1,
                "state"=>1,
                "monto"=>$monto,
            )) -> execute();
    echo "bx " .(time() - $time) ."<br>";
    $time=time();

    }*/


    db_query("INSERT personal_jornada_productividad (jid,pid,tipo,cajas,cajas_eq,diff_cajas,diff_eq,coeff,state,monto)
                  SELECT jid,pid,'resto',0,0,0,0,1,1,
                  CASE planta  
                    WHEN 1 THEN numeric_work*:valor_hora_1 
                    WHEN 2 THEN numeric_work*:valor_hora_2 
                    ELSE 0
                  END 
                  AS monto 
                  FROM planta_personal_jornada 
                  WHERE jid=:jid AND work_type IN ('varios','acomodador')", 
                        array(':jid'=>$jid,':valor_hora_1'=>$valor_hora[1],':valor_hora_2'=>$valor_hora[2])
              );

    db_query("UPDATE 
              planta_personal_jornada  ppj 
              JOIN personal p ON p.pid=ppj.pid
              JOIN jornadas j ON j.jid=ppj.jid
              LEFT JOIN valores_perido vp ON (vp.grupo='categorias' AND  p.categoria=vp.codigo 
                      AND j.start_time>=vp.start_time AND j.end_time<=vp.end_time)  
              SET ppj.monto = vp.valor
              WHERE ppj.jid=:jid
              ",array(':jid'=>$jid));


}

function ajuste_productividad($form,&$form_state,$jid,$pid){

    $coeff=db_select('jornada_personal_ajustes','a')->fields('a',array('coeff'))->condition('pid',$pid)->condition('jid',$jid)->execute()->fetchField();


    $persona=entity_load('data_personal',array($pid));
    $jornada=entity_load('data_jornadas',array($jid));
    drupal_set_title("Ajustes para {$persona[$pid]->name} para el dia ".format_date($jornada[$jid]->start_time,'custom','d/m/Y'));
    $form['coeff']=array('#type'=>'textfield', '#title'=>'Coeficciente','#default_value'=>($coeff)?$coeff:0);
    $form['pid']=array('#type'=>'value', '#value'=>$pid);
    $form['jid']=array('#type'=>'value', '#value'=>$jid);

    $form['enviar']=array('#type'=>'submit', '#value'=>'procesar');
    return $form;

}
function ajuste_productividad_submit($form,&$form_state){

     db_merge('jornada_personal_ajustes')
      ->key(
        array(
          'pid' => $form['pid']['#value'],
          'jid' => $form['jid']['#value'],
          )
        )
      ->fields(array(
          'pid' => $form['pid']['#value'],
          'jid' => $form['jid']['#value'],
          'coeff' => $form_state['values']['coeff'],
      ))
      ->execute();
      productividad_import_jid($form['jid']['#value']);
      drupal_goto('jornadas-laborales/productividad/'. $form['jid']['#value']);
}

function productividad_import_day($jid){
    
    productividad_import_jid($jid);
    productividad_planta_import_jid($jid);
    set_planta_personal_jornada($jid);
    set_productividad_jornada_otros($jid);
      $table=array();
    $rows=db_select('personal_jornada_productividad','jp')->fields('jp')->condition('jid',$jid)->execute();
    while ( $r = $rows->fetchAssoc()) {
        if(!@$table['header'])$table['header']=array_keys($r);

        $table['rows'][]=array_values($r);
    }
    $return= theme('table',$table);

    $table=array();
    $rows=db_select('planta_jornada_productividad','jp')->fields('jp')->condition('jid',$jid)->execute();
    while ( $r = $rows->fetchAssoc()) {
        if(!@$table['header'])$table['header']=array_keys($r);

        $table['rows'][]=array_values($r);
    }
    $return.= theme('table',$table);
    return $return;
}


function productividad_import_jid($jid){


    $jornada=array_shift(entity_load('data_jornadas',array($jid)));
    $coefficiene_ajustes=coefficientes_ajuste_jornada($jid);
    $valores=get_valores_for_time($jornada->start_time);

    $prod_pera=$valores['volumenes']['pera'];
    $prod_mzn=$valores['volumenes']['mnz'];

    $date_string=format_date($jornada->start_time,'custom','Y-m-d');
    $personal=all_personal_by_legajo();
    

    $result=db_query("SELECT
          `env`.`ESPECIE`  AS  especie,
        DATE(INGRESO) AS dia, 
          `e`.`LEGAJO`    AS legajo,
          `e`.`NOMBRE`    AS nombre,

          SUM(`emb`.`EQ`)      AS cajas,
          SUM(`emb`.`EQ` * `env`.`EQ`) AS cajas_eq,
          IF(max(`env`.`ESPECIE`) =1, {$prod_mzn},{$prod_pera}) AS prod_esperada,
          (SUM(`emb`.`EQ` ) - IF(`env`.`ESPECIE` =1, {$prod_mzn},{$prod_pera})) AS diff_cajas,
          (SUM(`emb`.`EQ` * `env`.`EQ`) - IF(`env`.`ESPECIE` =1, {$prod_mzn},{$prod_pera})) * AVG(`env`.`EQ`) AS diff_eq,
           
          `e`.`TURNO`     AS `turno`,
          `c`.`LINEA`     AS `linea`
        FROM (((`fruitid`.`cajas` `c`
             JOIN `fruitid`.`embaladores` `e`
               ON ((`e`.`EMBALADOR` = `c`.`EMBALADOR`)))
            JOIN `fruitid`.`envases` `env`
              ON ((`c`.`ENVASE` = `env`.`ENVASE`)))
           JOIN `fruitid`.`embalados` `emb`
             ON ((`c`.`EMBALADO` = `emb`.`EMBALADO`)))
        WHERE ((`c`.`EMPRESA` = 1)
               AND (`c`.`PALLET` >= 0))
        AND DATE(INGRESO)='".$date_string."'
         GROUP BY DATE(INGRESO), e.EMBALADOR ");
// quito el , `env`.`ESPECIE`
    $procesados=array();
    db_delete('personal_jornada_productividad')->condition('jid',$jornada->jid)->execute();
    foreach ($result as $item){
        //si no existe el personal creo un warning
        if(!isset($personal[$item->legajo])) {
            $pid=productividad_import_personal($item->legajo,$item->nombre);
            $personal[$item->legajo]=new stdClass();
            $personal[$item->legajo]->pid=$pid;

            drupal_set_message("El Legajo {$item->legajo} No existe en el sistema y fue creado",'error');
        }
        $ajuste=(@$coefficiene_ajustes[$personal[$item->legajo]->pid])?$coefficiene_ajustes[$personal[$item->legajo]->pid]:1;

        if(!isset($procesados[$item->legajo])){
            
            if($ajuste!=1){
                $diff=$item->cajas_eq + $item->cajas_eq *$ajuste;
                echo $diff;
                $monto=($diff>$item->prod_esperada)?($item->prod_esperada-$item->cajas_eq)*$valores['montos']['porcaja']:0;                


            } else {
                $monto=($item->diff_eq>0)?$item->diff_eq*$valores['montos']['porcaja']:0;
            }
            $procesados[$item->legajo]=$item;

            db_insert('personal_jornada_productividad')->
                fields(array(
                        "jid"=>$jornada->jid,
                        "pid"=>$personal[$item->legajo]->pid,
                        "tipo"=>'embalador',
                        "cajas"=>$item->cajas,
                        "cajas_eq"=>$item->cajas_eq,
                        "diff_cajas"=>$item->diff_cajas,
                        "diff_eq"=>$item->diff_eq,
                        "coeff"=>$ajuste,
                        "state"=>1,
                        "monto"=>$monto,
                    )) -> execute();
           db_update('personal_jornada')->fields(array('work_type' =>'Embalador' ,'planta' =>$item->linea ))->condition('jid',$jornada->jid)->condition('pid',$personal[$item->legajo]->pid)->condition('work_type','nn')->execute();

        } else{

/*
            $total=$item->cajas+$procesados[$item->legajo]->cajas;
            $primer_especie_porcentaje=($procesados[$item->legajo]->cajas*1)/$total;
            $segunda_especie_porcentaje=($item->cajas*1)/$total;
            $item->prod_esperada=($item->prod_esperada * 1
            //$item->cajas_eq+=$procesados[$item->legajo]->cajas_eq;
            //$total-> 100
            //$to
            
            $item->cajas+=$procesados[$item->legajo]->cajas;            
            $item->cajas_eq+=$procesados[$item->legajo]->cajas_eq;*/            
            drupal_set_message("El Legajo {$item->legajo} Tiene varias especies ",'error');

            $procesados[$item->legajo]=$item;

        }

    
    }

    
}

function productividad_planta_import_jid($jid){


    $jornada=array_shift(entity_load('data_jornadas',array($jid)));
    $valores=get_valores_for_time($jornada->start_time);
    $prod_pera=$valores['volumenes']['pera'];
    $prod_mzn=$valores['volumenes']['mnz'];


    $date_string=format_date($jornada->start_time,'custom','Y-m-d');
    
    $result=db_query("SELECT
          `env`.`ESPECIE`  AS  especie,
        DATE(INGRESO) AS dia, 
    COUNT(DISTINCT `c`.`EMBALADOR`) AS embaladores,
          SUM(`emb`.`EQ`)      AS cajas,
          SUM(`emb`.`EQ` * `env`.`EQ`) AS cajas_eq,
          IF(MAX(`env`.`ESPECIE`) =1, {$prod_mzn},{$prod_pera})* COUNT(DISTINCT `c`.`EMBALADOR`) AS prod_esperada,
          (SUM(`emb`.`EQ` ) - IF(MAX(`env`.`ESPECIE`) =1, {$prod_mzn},{$prod_pera})* COUNT(DISTINCT `c`.`EMBALADOR`)) AS diff_cajas,
          (SUM(`emb`.`EQ` * `env`.`EQ`) - ( IF(MAX(`env`.`ESPECIE`) =1, {$prod_mzn},{$prod_pera})* COUNT(DISTINCT `c`.`EMBALADOR`))) * AVG(`env`.`EQ`) AS diff_eq,
           
          `e`.`TURNO`     AS `turno`,
          `c`.`LINEA`     AS `linea`
        FROM (((`fruitid`.`cajas` `c`
             JOIN `fruitid`.`embaladores` `e`
               ON ((`e`.`EMBALADOR` = `c`.`EMBALADOR`)))
            JOIN `fruitid`.`envases` `env`
              ON ((`c`.`ENVASE` = `env`.`ENVASE`)))
           JOIN `fruitid`.`embalados` `emb`
             ON ((`c`.`EMBALADO` = `emb`.`EMBALADO`)))
        WHERE ((`c`.`EMPRESA` = 1)
               AND (`c`.`PALLET` >= 0))
        AND DATE(INGRESO)='".$date_string."'
         GROUP BY DATE(INGRESO), c.linea");
// quito el , `env`.`ESPECIE`

    $procesados=array();
    db_delete('planta_jornada_productividad')->condition('jid',$jornada->jid)->execute();
    foreach ($result as $item){
        //si no existe el personal creo un warning

            
            $monto=($item->diff_eq>0)?$item->diff_eq*$valores['montos']['porcajaresto']:0;

            db_insert('planta_jornada_productividad')->
                fields(array(
                        "jid"=>$jornada->jid,
                        "planta"=>$item->linea,
                        "cajas"=>$item->cajas,
                        "cajas_eq"=>$item->cajas_eq,
                        "diff_cajas"=>$item->diff_cajas,
                        "diff_eq"=>$item->diff_eq,
                        "coeff"=>$ajuste,
                        "state"=>1,
                        "cant"=>$item->embaladores,

                        "monto"=>$monto,
                    )) -> execute();
    
    }



}




function productividad_import_personal($legajo,$name){
  
    return db_merge('personal')
      ->key(
        array(
          'legajo' => intval($legajo)
          )
        )
      ->fields(array(
            'legajo' => intval($legajo),
            'name' => $name,
            'active' => 1,
            'change_date' => time(),
            'create_date' => time(),
            'create_uid' => 1
      ))
      ->execute();
  
}




function reglas_coeficiente($kilos){
/*
a) => 15 = 23 ► equivale 1
b) => 12 <15 ► equivale 0,73
c) => 10 <12 ► equivale 0,66
d) => 5 < 10 ► equivale 0,50
e) => 1 < 5 ► equivale 0,39

*/
    if($kilos < 5){
        return 0.39;
    } else 
    if( $kilos > 5 && $kilos < 10){
        return 0.5;
    } else
    if( $kilos > 10 && $kilos < 12){
        return 0.66;
    } else
    if( $kilos > 12 && $kilos < 15){
        return 0.73;
    }
    if( $kilos > 15 ){
        return 0.73;
    }

}

function min_prod_volume($specie){
    switch ($specie) {
        case 1:
        case 'manzana':
            return 114;
            break;
        
        case 2:
        case 'pera':
            return 96;
            break;
    }
}
function coefficientes_ajuste_jornada($jid){
    return db_select('jornada_personal_ajustes','a')->fields('a',array('pid','coeff'))->condition('jid',$jid)->execute()->fetchAllKeyed();
}

function detalle_embalador($jid,$pid){


    $persona=entity_load('data_personal',array($pid));
    $jornada=entity_load('data_jornadas',array($jid));

    drupal_set_title("{$persona[$pid]->legajo} {$persona[$pid]->name} " . format_date($jornada[$jid]->start_time, 'custom','d/m/Y'));

    $daystamp=floor($jornada[$jid]->start_time/UN_DIA);
    
    $query=db_select('personal_jornada','p');
    $query->join('jornadas','j','j.jid=p.jid');

    $query->fields('p',array('start_time','end_time','work_type','planta','time_total'))->condition('daystamp',$daystamp)->condition('p.pid',$pid);
    $result=$query->execute();
    
    $table=array();
    $table['header']=array('Inicio','Final','tarea','Planta','Duracion');
    $tt=0;
    while ( $item = $result->fetchObject()){

        $table['rows'][]=array(format_date($item->start_time,'custom','G:i'),format_date($item->end_time,'custom','G:i'),$item->work_type,$item->planta,format_interval($item->time_total));
        $tt=$tt+$item->time_total;
    }
        $table['rows'][]=array('','','','Total',format_interval($tt));

    $return.= theme('table',$table);


    $date_string=format_date($jornada[$jid]->start_time,'custom','Y-m-d');
    $legajo=$persona[$pid]->legajo;

    $result=db_query("SELECT
          `env`.`ESPECIE`  AS  especie,
        concat(HOUR(MIN(INGRESO)),':',lpad(MINUTE(MAX(INGRESO)),2,'0')) AS primer, 
        concat(HOUR(MAX(INGRESO)),':',lpad(MINUTE(MAX(INGRESO)),2,'0')) AS ultima, 

        env.nombre AS envase,
        emb.nombre AS embalado,
        
          SUM(`emb`.`EQ`)      AS cajas,
          SUM(`emb`.`EQ` * `env`.`EQ`) AS cajas_eq,
           
          `c`.`LINEA`     AS `linea`
        FROM (((`fruitid`.`cajas` `c`
             JOIN `fruitid`.`embaladores` `e`
               ON ((`e`.`EMBALADOR` = `c`.`EMBALADOR`)))
            JOIN `fruitid`.`envases` `env`
              ON ((`c`.`ENVASE` = `env`.`ENVASE`)))
           JOIN `fruitid`.`embalados` `emb`
             ON ((`c`.`EMBALADO` = `emb`.`EMBALADO`)))
        WHERE ((`c`.`EMPRESA` = 1)
               AND (`c`.`PALLET` >= 0))
        AND DATE(INGRESO)='{$date_string}' AND legajo={$legajo}
         GROUP BY DATE(INGRESO), e.EMBALADOR, `env`.ESPECIE,`env`.`ENVASE`, `emb`.`EMBALADO`
        order by ingreso

         ");

    $table=array();
    $cajas=0;
    $cajaEq=0;
    foreach ($result as $item){
        if(!@$table['header'])$table['header']=array_keys((array)$item);
        $cajas=$cajas+$item->cajas;
        $cajaEq=$cajaEq+$item->cajas_eq;
        $table['rows'][]=array_values((array)$item);
    }
        $table['rows'][]=array('','','','','',$cajas,$cajaEq);

    $return.= theme('table',$table);


    return $return;

}

/*SELECT e.EMBALADOR , e.legajo, e.nombre , COUNT(*) AS CANTIDAD,
 emb.eq  AS ENV_EQ, env.eq AS  EMB_EQ, 000000.00 AS CAN_EQ , e.DNI AS DNI,
 env.NOMBRE AS ENVASE, emb.NOMBRE AS EMBALADO , ingreso, empresa , e.turno
FROM CAJAS c
JOIN embaladores e ON (e.embalador=c.embalador)
JOIN envases env ON (c.envase = env.envase)
JOIN embalados emb ON (c.embalado = emb.embalado) 
 WHERE EMPRESA = 1
 AND INGRESO > "2015-04-27 00:00:00"
 AND INGRESO < "2015-04-29 00:00:00"
 AND PALLET >= 0
GROUP BY c.EMBALADOR, c.ENVASE, c.EMBALADO
*/