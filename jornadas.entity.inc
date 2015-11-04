<?php
function jornadas_entity_load($entities, $type) {
    if($type=='data_jornadas'){

        $result=db_select('jormada_tiempo_produccion','tp')->fields('tp')->condition('jid',array_keys($entities))->execute();
        while ( $item = $result->fetchObject()) {
            $entities[$item->jid]->production_time[$item->lid][$item->fid]=$item->time;
        }
 //       $entities[]
    }
}

function jornadas_entity_update($entity, $type) {
  // Update the entity's entry in a fictional table of all entities.
 if($type=='data_jornadas'){
     db_delete('jormada_tiempo_produccion')->condition('jid',$entity->jid)->execute();
     
     foreach ($entity->production_time as $lid => $element) {
         foreach ($element as $fid => $time) {
            db_insert('jormada_tiempo_produccion')->fields(array('jid'=>$entity->jid,'lid'=>$lid,'fid'=>$fid,'time'=>$time))->execute();

         }
     }
 }
  

}

function jornadas_entity_insert($entity, $type) {
  // Update the entity's entry in a fictional table of all entities.
 if($type=='data_jornadas'){
    if (!isset($entity->production_time )) return ;
     foreach ($entity->production_time as $lid => $element) {
         foreach ($element as $fid => $time) {
            db_insert('jormada_tiempo_produccion')->fields(array('jid'=>$entity,'lid'=>$lid,'fid'=>$fid,'time'=>$time))->execute();
         }
     }
 }
  

}