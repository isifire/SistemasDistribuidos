package cliente;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

// Imports necesarios para RabbitMQ
import com.rabbitmq.client.Channel;
import com.rabbitmq.client.Connection;
import com.rabbitmq.client.ConnectionFactory;

// Imports necesarios para RMI
import java.io.IOException;
import java.rmi.Naming;

public class Cliente {
    private final static String NOMBRE_COLA = "cola";  // A RELLENAR (cambiar nombre a cola)

    static int id; // Identificador del cliente

    public static void main(String[] argv) throws Exception {

        // =================================================
        // Obtener argumentos de línea de comandos
        if (argv.length < 3) {
            System.out.println("Uso: cliente <id_cliente> <fichero_consultas> <fichero_respuestas>");
            System.exit(1);
        }

        try {
            id = Integer.parseInt(argv[0]);
        } catch (NumberFormatException e) {
            System.out.println("El id del cliente debe ser entero");
            return;
        }

        // =================================================
        // Parte principal, toda dentro de un try para capturar cualquier excepción
        try {

            // Arrancar el la parte servidor RMI del cliente y registrarlo
            // ante rmiregistry con el nombre adecuado a este cliente
            // A RELLENAR:
            ClienteImpl cli = new ClienteImpl();
            Naming.rebind("rmi://localhost/Cliente" + id, cli);



            // Conectar con Rabbit para poder enviar peticiones a la cola
            // A RELLENAR:
            ConnectionFactory factory = new ConnectionFactory();
            factory.setHost("localhost");
            Connection connection = factory.newConnection();
            Channel channel = connection.createChannel();
            
            channel.queueDeclare(NOMBRE_COLA, false, false, false, null);



            // Realizar el envio de los eventos
            enviar_consultas(channel, cli, argv[1], argv[2]);

            // Terminar
            System.out.println("Cliente finalizado");
            channel.close();
            connection.close();
            System.exit(0);
        } catch (Exception e) {
            // Cualquier excepción simplemente se imprime
            System.out.println("Error en Cliente" + e.getMessage());
            e.printStackTrace();
        }
    }

    // =========================================================================
    // La función enviar_consultas lee del fichero de entrada y envia las líneas leídas
    // como mensajes a través de la cola de mensajes
    // Requiere como parámetros:
    //
    // - El canal de comunicación con RabbitMQ para enviar los mensajes
    // - La referencia al objeto RMI exportado por el cliente
    // - El nombre del fichero con las consultas a enviar
    // - El nombre del fichero donde se volcarán los resultados
    //
    // Una vez finaliza de leer todos los mensajes del fichero de entrada
    // y enviarlos a la cola, la función retorna
    static void enviar_consultas(Channel channel, ClienteImpl cli, String fich_consultas, String fich_resp)
            throws IOException, InterruptedException {
        BufferedReader br = new BufferedReader(new FileReader(fich_consultas));
        BufferedWriter bw = new BufferedWriter(new FileWriter(fich_resp));
        String msg;       // Para componer los mensajes a enviar por Rabbit
        String respuesta; // Respuesta a la petición realizada
        String linea;

        try {
            String query = br.readLine();

            while (query != null) {
                // Aqui es donde hay que enviar la consulta y esperar la respuesta
                // por RMI

                // La consulta es primero el nombre del cliente registrado en RMI
                // seguido de la consulta leida del fichero, separados ambos por una coma
                // y sin espacios. Este mensaje se envía a la cola Rabbit

                // A RELLENAR:
                msg = "Cliente" + id + "," + query;
                channel.basicPublish("", NOMBRE_COLA, null, msg.getBytes());
                System.out.println(" [x] Sent '" + msg + "'");



                // Se espera la respuesta que llegará por rmi y cuando se reciba se
                // escribe en el fichero de respuestas
                // A RELLENAR:
                respuesta = cli.getRespuesta();
                bw.write(respuesta);
                bw.newLine();

                // Leer la siguiente consulta del fichero
                query = br.readLine();
            }
        } catch (Exception e) {
            // Cualquier excepción simplemente se imprime
            System.out.println("Error en Cliente" + e.getMessage());
            e.printStackTrace();
        } finally {
            // Cerrar fichero de entrada y de salida
            br.close();
            bw.close();
        }

    }
}
