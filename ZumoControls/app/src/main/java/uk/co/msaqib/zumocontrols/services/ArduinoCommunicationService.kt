package uk.co.msaqib.zumocontrols.services

import android.app.Service
import android.bluetooth.BluetoothSocket
import android.content.Intent
import android.os.Binder
import android.os.IBinder
import android.os.SystemClock
import uk.co.msaqib.zumocontrols.Interfaces.IServiceCallbacks
import java.io.IOException
import java.io.InputStream
import java.io.OutputStream
import java.lang.Exception
import java.sql.Timestamp
import java.time.Instant
import java.util.*


class ArduinoCommunicationService : Service() {

    // Binder given to clients
    private val binder = LocalBinder()
    private lateinit var mBTSocket: BluetoothSocket
    private lateinit var listener: IServiceCallbacks
    private lateinit var mCommunicationThread: CommunicationThread
    private var isThreadAlive = false
    private var mPlayer: String = ""
    /**
     * Class used for the client Binder.  Because we know this service always
     * runs in the same process as its clients, we don't need to deal with IPC.
     */
    inner class LocalBinder : Binder() {
        // Return this instance of LocalService so clients can call public methods
        fun getService(): ArduinoCommunicationService = this@ArduinoCommunicationService
    }

    override fun onBind(intent: Intent): IBinder {
        return binder
    }

    fun startBTConnection(socket: BluetoothSocket, listener: IServiceCallbacks, player: String){
        this.listener = listener
        try{
            mPlayer = player
            mBTSocket = socket
            mBTSocket.connect()
            mCommunicationThread = CommunicationThread(mBTSocket,listener)
            mCommunicationThread.start()
            listener.updateStatus(mBTSocket.isConnected,if(mBTSocket.isConnected) "Connected" else "Connection Failed")
        }catch (e: Exception){
            listener.onError(e.localizedMessage)
            listener.updateStatus(false,"Connection Failed")
        }
    }

    fun writeToArduino(data: String){
        if(::mCommunicationThread.isInitialized){
            mCommunicationThread.write(data);
        }
    }

    fun stopZumo(){
        this.writeToArduino("STOP:0|0;")
    }

    fun startAutoMode(){
        this.writeToArduino("ZUMO|AUTO:200|200;")
    }


    fun flush(){
        if(::mCommunicationThread.isInitialized){
            mCommunicationThread.flush();
        }
    }

    fun closeConnection(){
        try {
            if(::mBTSocket.isInitialized){
                mBTSocket.close()
                isThreadAlive = false
            }else{
                listener.onError("Connection not initialised!")
            }
        } catch (e: IOException) {
            listener.onError(e.localizedMessage)
        }
    }

    private inner class CommunicationThread(mmSocket: BluetoothSocket, listener: IServiceCallbacks) : Thread() {
        private val mmInStream: InputStream = mmSocket.inputStream
        private val mmOutStream: OutputStream = mmSocket.outputStream


        override fun run() {
            val buffer = ByteArray(1024)  // buffer store for the stream
            var bytes: Int // bytes returned from read()
            // Keep listening to the InputStream until an exception occurs
            isThreadAlive = true
            while (isThreadAlive) {
                try {
                    // Read from the InputStream
                    bytes = mmInStream.available()
                    if (bytes != 0) {
                        SystemClock.sleep(100) //pause and wait for rest of data. Adjust this depending on your sending speed.
                        bytes = mmInStream.available() // how many bytes are ready to be read?
                        bytes = mmInStream.read(buffer, 0, bytes) // record how many bytes we actually read
                        listener.onDataReceived(buffer)
                    }
                } catch (e: IOException) {
                   listener.onError(e.localizedMessage)
                }

            }
        }

        /* Call this from the main activity to send data to the remote device */
        fun write(input: String) {
            val bytes = input.toByteArray()           //converts entered String into bytes
            try {
                mmOutStream.write(bytes)
            } catch (e: IOException) {
                listener.onError(e.localizedMessage);
                if(e.cause == null){
                    closeConnection()
                    listener.updateStatus(false, "Disconnected")
                }
            }

        }

        fun flush(){
            try{
                mmOutStream.flush();
            }catch (e: IOException){
                listener.onError(e.localizedMessage);
            }
        }
    }


}
