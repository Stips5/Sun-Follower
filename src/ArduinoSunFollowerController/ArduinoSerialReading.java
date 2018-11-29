package ArduinoSunFollowerController;

import com.fazecast.jSerialComm.*;

import javax.swing.*;
import java.util.Scanner;

public class ArduinoSerialReading {

    public static void main(String argv[])
    {
       serialCommunication();

    }


    public static void serialCommunication()
    {
        SerialPort ports[] = SerialPort.getCommPorts();
        int chosenPort;

        System.out.println("Ports found: ");

        for (int i = 0; i < ports.length; i++) {

            System.out.println(i + ") " + ports[i].getSystemPortName());
        }

//        System.out.println("Select port ");

//        Scanner pick = new Scanner(System.in);
//        chosenPort = pick.nextInt();
        chosenPort = 0;


        SerialPort port = ports[chosenPort];
        port.setComPortParameters(9600,8,1,0);

        port.openPort();
        if(port.isOpen())
        {
            System.out.println("Arduino connected on port " + ports[chosenPort].getSystemPortName());

            port.setComPortTimeouts(SerialPort.TIMEOUT_SCANNER,0,0);

            Scanner inputRead = new Scanner(port.getInputStream());

            //waits for input from serial
            while(inputRead.hasNextLine())
            {
                System.out.println(inputRead.nextLine());
            }

        }
        else
        {
            System.out.println("Port " + ports[chosenPort].getSystemPortName() + " not oppened");
        }

        port.closePort();

    }


    private static int portPick(SerialPort portovi[],JComboBox<String> menuList)
    {
        int selected = 0;

        System.out.print("Chose port:\n");

        for (int i = 0; i < portovi.length; i++) {
            System.out.println(portovi[i].getSystemPortName());
            menuList.addItem(portovi[i].getSystemPortName());
        }



        Scanner scan = new Scanner(System.in);
        selected = scan.nextInt();

        return selected;
    }
}
